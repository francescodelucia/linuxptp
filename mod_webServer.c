
#include "threadpool/threadpool.h"
#include "utils.h"
#include "string_util.h"

#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include "mod_webServer.h"
#include "mod_res.h"
#include "mod_udpServer.h"


const char* WEB_GET = "GET";
const char* CGI_BIN_PATH = "/cgi-bin/";
const int MAX_CWD = 100;

//int struct_size=0;

extern void get_live_data(char *result);
pthread_t tid_web;


void writeln_to_socket(int sockfd, const char *message) {
  write(sockfd, message, strlen(message));
  write(sockfd, "\r\n", 2);
}


void write_content_to_socket(int sockfd, const char *content,int content_type) {
  char length_str[100];
  sprintf(length_str, "%d", (int)strlen(content));

  char *content_length_str = concat("Content-Length: ", length_str);
  writeln_to_socket(sockfd, "Server: PetkoWS/1.0 (MacOS)");
  switch(content_type){
    case CONTENT_TYPE_TEXT:
      writeln_to_socket(sockfd, "Content-Type: text/html");
    break;
    case CONTENT_TYPE_JSON:
      writeln_to_socket(sockfd, "Content-Type: application/json");
    break;
    default:
      writeln_to_socket(sockfd, "Content-Type: text/html");
    break;
  }
  writeln_to_socket(sockfd, content_length_str);
  writeln_to_socket(sockfd, "");
  writeln_to_socket(sockfd, content);

  free(content_length_str);
}

void http_404_reply(int sockfd) {
  writeln_to_socket(sockfd, "HTTP/1.1 404 Not Found");

  static const char *content = "<html><body><h1>Not found</h1></body></html>\r\n";
  write_content_to_socket(sockfd, content,CONTENT_TYPE_TEXT);
}

void http_get_reply(int sockfd, const char *content,int content_type) {
  writeln_to_socket(sockfd, "HTTP/1.1 200 OK");
  write_content_to_socket(sockfd, content,content_type);
}


int is_get(char *text) {
  return starts_with(text, WEB_GET);
}

char *get_path(char *text) {
  int beg_pos = strlen(WEB_GET) + 1;
  char *end_of_path = strchr(text + beg_pos, ' ');
  int end_pos = end_of_path - text;

  int pathlen = end_pos - beg_pos;
  char *path = malloc(pathlen + 1);
  substr(text, beg_pos, pathlen, path);
  path[pathlen] = '\0';

  return path;
}

int is_cgi_bin_request(const char *path) {
  if (contains(path, "/cgi-bin/")) return 1;
  return 0;
}

char *read_file(FILE *fpipe) {
  int capacity = 10;
  char *buf = malloc(capacity);
  int index = 0;

  int c;
  while ((c = fgetc(fpipe)) != EOF) {
    assert(index < capacity);
    buf[index++] = c;

    if (index == capacity) {
      char *newbuf = malloc(capacity * 2);
      memcpy(newbuf, buf, capacity);
      free(buf);
      buf = newbuf;
      capacity *= 2;
    }
  }
  
  buf[index] = '\0';
  return buf;
}

struct request_pair {
  char *path;
  char *query;
};

struct request_pair extract_query(const char *cgipath_param) {
  struct request_pair ret;
  char *qq = strchr(cgipath_param, '?');

  if (qq == NULL) {
    ret.path = strdup(cgipath_param);
    ret.query = NULL;
  } else {
    int path_len = qq - cgipath_param;
    ret.path = malloc(path_len + 1);
    strncpy(ret.path, cgipath_param, path_len);
    ret.path[path_len] = 0;

    int query_len = strlen(cgipath_param) - path_len - 1;
    ret.query = malloc(query_len + 1);
    const char* query_start_pos = cgipath_param + path_len + 1;
    strncpy(ret.query, query_start_pos, query_len);
    ret.query[query_len] = '\0';
  }

  return ret;
}

void run_cgi(int sockfd, const char *curdir, const char *cgipath_param) {
  char *fullpath;
  struct request_pair req = extract_query(cgipath_param);

  char *params;
  if (req.query) { 
    params = malloc(strlen(req.query) + 100);
    sprintf(params, "QUERY_STRING='%s' ", req.query);
  } else {
    params = strdup("");
  }

  if (ends_with(req.path, ".py")) {
    // TODO: Overflow possible?
    fullpath = concat4(params, "python ", curdir, req.path);
  } else {
    fullpath = concat3(params, curdir, req.path);
  }

  free(params);
  free(req.path);
  free(req.query);

  //printf("Executing: [%s]\n", fullpath);

  FILE *fpipe = popen(fullpath, "r");
  free(fullpath);

  if (!fpipe) {
    perror("Problem with popen");
    http_404_reply(sockfd);
  } else {
    char* result = read_file(fpipe);
    http_get_reply(sockfd, result,CONTENT_TYPE_TEXT);
    free(result);
  }
}

void output_embedded_html(int sockfd, const char *curdir, const char *path,int content_type) {    
  int str = strcmp(path, "\/");
  if ( strcmp(path, "\/") == 0) {    
    http_get_reply(sockfd, html,content_type);    
  }else{        
    http_404_reply(sockfd);
  } 
}


void output_live_data(int sockfd) {
  char *result=malloc(MAX_SIZE*MAX_SIZE);  
  get_live_data(result);  
  http_get_reply(sockfd, result,CONTENT_TYPE_JSON);      
  free(result);
}



void *handle_socket_thread(void* sockfd_arg) {
  int sockfd = *((int *)sockfd_arg);
  
  char *text = read_text_from_socket(sockfd);  

  if (is_get(text)) {
    char curdir[MAX_CWD];

    if (!getcwd(curdir, MAX_CWD)) {
      error("Couldn't read curdir");
    }

    char *path = get_path(text);

    if (is_cgi_bin_request(path)) {
      run_cgi(sockfd, curdir, path);
    } else {      
      if (strcmp(path, "\/live") == 0) {
        output_live_data(sockfd);
      }else{
        output_embedded_html(sockfd, curdir, path, CONTENT_TYPE_TEXT);
      }
    }

    free(path);
  } else {
    // The server only supports GET.
    http_404_reply(sockfd);
  }

  free(text);
  close(sockfd);
  free(sockfd_arg);

  return NULL;
}

int create_listening_socket(uint16_t port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }
  int setopt = 1;

  // Reuse the port. Otherwise, on restart, port 8000 is usually still occupied for a bit
  // and we need to start at another port.
  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&setopt, sizeof(setopt))) {
    error("ERROR setting socket options");
  }

  struct sockaddr_in serv_addr;
  
  while (1) {
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      port++;
    } else {
      break;
    }
  }

  if (listen(sockfd, SOMAXCONN) < 0) error("Couldn't listen");
  //printf("Running on port: %d\n", port);

  return sockfd;
}

void* webThread(void* arg) {     
  int port=(uint16_t)arg;      
    int sockfd = create_listening_socket(port); 
  int i=0;  
  
  struct thread_pool* pool = pool_init(4);
  struct sockaddr_in client_addr;
  int cli_len = sizeof(client_addr);

  while (1) {
    int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, (socklen_t *) &cli_len);
    if (newsockfd < 0) error("Error on accept");
    
    int *arg =(int*)malloc(sizeof(int));
    *arg = newsockfd;
    pool_add_task(pool, handle_socket_thread, arg);
  }
  close(sockfd);

  return 0;
}

int webServer(u_int16_t port) {   
  int err = pthread_create(&tid_web, NULL, &webThread,port);  

  if (err != 0){ 
    printf("\ncan't create web thread :[%s]", strerror(err));
    return 0;
  }else{
    //printf("\n Thread created successfully\n");    
  }
  return 1;  
}
