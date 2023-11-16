#pragma once
#ifndef __MOD_WEB_SERVER__
    #define __MOD_WEB_SERVER__
    
#ifdef  __cplusplus
extern "C" {
#endif


struct request_pair;
extern void writeln_to_socket(int sockfd, const char *message);
extern void write_content_to_socket(int sockfd, const char *content,int content_type);
extern void http_404_reply(int sockfd);
extern void http_get_reply(int sockfd, const char *content,int content_type);  

extern int is_get(char *text);

extern char *get_path(char *text);

extern int is_cgi_bin_request(const char *path);

extern char *read_file(FILE *fpipe);

extern struct request_pair extract_query(const char *cgipath_param);

extern void run_cgi(int sockfd, const char *curdir, const char *cgipath_param);

extern void output_embedded_html(int sockfd, const char *curdir, const char *path,int content_type);

extern void output_live_data(int sockfd);

extern void *handle_socket_thread(void* sockfd_arg);

extern int create_listening_socket(uint16_t port);

extern void* webThread(void* arg);

extern int webServer(uint16_t port);

#ifdef  __cplusplus
}
#endif

#endif
