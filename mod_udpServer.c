#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <semaphore.h> 
//#include "mod_const.h"
#include "mod_memManager.h"
#include "mod_udpServer.h"





memBlock *_data_list;
serverData _sData;

int struct_size=0;

sem_t mutex; 
pthread_t tid;
pthread_t tid_refresh;

char _NetIp[20]={0};


void updateServerData(serverData *sD){

  if(strlen(sD->hostname)>0)
  {
    char host_name[256]={0};
    struct hostent *host_entry;
    int ihost;
    ihost = gethostname(host_name,256);
    host_entry = gethostbyname(host_name);  
    sprintf(sD->hostname,"%s",host_name);   
  }   
  sD->time = timeInMilliseconds();    
  
  if(*((uint32_t*)&sD->ip) == 0){
    char r[4];
    getIP(&r);          
    *((uint32_t*)&sD->ip) = *((uint32_t*)&r);
    /*sD->ip[0] = r[0];
    sD->ip[1] = r[1];
    sD->ip[2] = r[2];
    sD->ip[3] = r[3];*/
  }  
}

struct mod_config get_base_mod_config(){   
  struct mod_config m_conf;
  m_conf.port_www = 8000;
  m_conf.mode = NO_MODE;
  m_conf.udp_port = 8090;  
  memset(m_conf.remoteIP,-1,sizeof(int)*4);
  return m_conf;
}

int mod_parse_ip(const uint8_t * string,uint8_t string_length,int* result){    
    int ret[4]={0};
    uint8_t at_least_one_symbol = 0;
    uint8_t symbol, string_index = 0, result_index = 0;
    uint16_t data = 0;
    while ( string_index < string_length ) {
        symbol = string[string_index];
        if ( isdigit ( symbol ) != 0 ) {
            symbol -= '0';
            data   = data * 10 + symbol;
            if ( data > UINT8_MAX ) {
                // 127.0.0.256
                return ERROR_IPV4_DATA_OVERFLOW;
            }
            at_least_one_symbol = 1;
        } else if ( symbol == '.' ) {
            if ( result_index < 3 ) {
                if ( at_least_one_symbol ) {
                    ret[result_index] = data;
                    data = 0;
                    result_index ++;
                    at_least_one_symbol = 0;
                } else {
                    // 127.0..1
                    return ERROR_IPV4_NO_SYMBOL;
                }
            } else {
                // 127.0.0.1.2
                return ERROR_IPV4_INPUT_OVERFLOW;
            }
        } else {
            // 127.*
            return ERROR_IPV4_INVALID_SYMBOL;
        }
        string_index ++;
    }
    if ( result_index == 3 ) {
        if ( at_least_one_symbol ) {
            ret[result_index] = data;
            memcpy(result,ret,sizeof(int)*4);
            return 0;
        } else {
            // 127.0.0.
            return ERROR_IPV4_NOT_ENOUGH_INPUT;
        }
    } else {
        // result_index will be always less than 3
        // 127.0
        return ERROR_IPV4_NOT_ENOUGH_INPUT;
    }
}

char *get_ip_from_array(char *ip){
  char ret[20]={0};
  for(int k=0;k<4;k++){
    sprintf(ret,"%s%i.",ret,ip[k]);
  }
  if(ret[strlen(ret)-1]=='.'){
    ret[strlen(ret)-1]=0;  
  }
  //sprintf(result,"%s",ret);
  return &ret;
}

void printBufferFull(int size){  
  for(int k=0;k<(size);k++){
        for(int f=0;f<DMEMORY;f++){
          //printf("%i ",dataBuffer[k][f]);
        }
        printf("\n");                
        //printf("%s\n",dataBuffer[k]);
  }
} 
  
void printBufferSegment(int pos){
  printf("struct_pos %i ok!\n",pos);
  for(int k=0;k<DMEMORY;k++){        
    //printf("%i ",dataBuffer[pos][k]);        
  }
  printf("\n");                
  //printf("%s\n",dataBuffer[pos]);
} 

void printStringDebug(char *str){  
  //printf("printStringDebug\n");
  for(int k=0;k<strlen(str);k++){            
    printf("%i ",str[k]);    
  }
  printf("\n");                    
  printf("%s\n",str);
} 

void printMemDebug(char *str){  
  printf("printStringDebug\n");
  for(int k=0;k<DMEMORY;k++){            
    printf("%i ",(uint8_t)str[k]);    
  }
  printf("\n");                      
} 


void replace_char_from_string(char from, char to, char *str)
{
    int i = 0;
    int len = strlen(str)+1;
    for(i=0; i<len; i++)
    {
        if(str[i] == from)
        {
            str[i] = to;
        }
    }
}

void parseData(char* buff,void *_data){    
  char * token = strtok(buff, ":");
  int k =0;
  while( token != NULL ) {
        switch (k){
          case 0:            
            if(token[strlen(token)-1] == 10){token[strlen(token)-1]=0;}
            sprintf(((memBlock*)_data)->ip_received,"%s",token);            
          break;
          case 1:                        
            if(token[strlen(token)-1] == 10){token[strlen(token)-1]=0;}            
            sprintf(((memBlock*)_data)->time_,"%s",token);                        
          break;        
        }      
        token = strtok(NULL, " ");
        k++;
  }  
}

memBlock  *findIp(void*_data){       
  return searchElem(_data_list,_data);  
}

long long timeInMilliseconds(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void getIP(unsigned char *out ){
  
  unsigned char ipstr[15] = {0};
  char *marker, *ret;
  unsigned char ipout[4];
  //unsigned char out[4]={0};
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  ifr.ifr_addr.sa_family = AF_INET;

  strncpy(ifr.ifr_name, "ens33", IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  char ip[20]={0};
  sprintf(ip,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

  if(ip!=NULL){      
      memcpy(ipstr,ip,sizeof(unsigned char)*15);
      ret = strtok_r(ipstr, ".", &marker);      
      ipout[0] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);      
      ipout[1] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);      
      ipout[2] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);      
      ipout[3] = (char)strtod(ret, NULL);            
      memcpy(out,&ipout,sizeof(unsigned char)*4);  
    }else{
      memset(out,0,4);
  }
}

const char * get_ip()
{
    // Read out "hostname -I" command output
    FILE *fd = popen("hostname -I", "r");
    if(fd == NULL) {
    fprintf(stderr, "Could not open pipe.\n");
    return NULL;
    }
    // Put output into a string (static memory)
    static char buffer[256];
    fgets(buffer, 256, fd);

    // Only keep the first ip.
    for (int i = 0; i < 256; ++i)
    {
        if (buffer[i] == ' ')
        {
            buffer[i] = '\0';
            break;
        }
    }

    char *ret = malloc(strlen(buffer) + 1);
    memcpy(ret, buffer, strlen(buffer));
    ret[strlen(buffer)] = '\0';
    //printf("%s\n", ret);
    return ret;
}

void get_live_data(char *result ) {        
  sprintf(result,"{\"data\":[");  
  serverData _sD;
  memBlock *mmB=calloc(sizeof(memBlock*),1);  
  printf("get_live_data #1\n");  
  mmB = _data_list; 
  //printf("get_live_data %i\n",mmB->ip_received[0]);  
  while (mmB != NULL) {            
    sprintf(result,
      "%s{\"IP_CLIENT\":\"%i.%i.%i.%i\",\"TIME\":\"%llu\",\"DIFF_SERVER\":\"%llu\",\"FRESH_DATA\":\"%i\"},",
      result,
      (u_int8_t)mmB->ip_received[0],
      (u_int8_t)mmB->ip_received[1],
      (u_int8_t)mmB->ip_received[2],
      (u_int8_t)mmB->ip_received[3],          
      mmB->time_,
      abs(mmB->time_-_sD.time),
      mmB->fresh_time
    );          
    //mmB =  mmB->next;        
    memcpy(mmB,mmB->next,sizeof(memBlock*)); 
    printf("get_live_data #888 %s\n",result);
  }     
  printf("get_live_data #4\n");
  if(result[strlen(result)-1]==','){
    result[strlen(result)-1]=0;  
  }  
  printf("get_live_data #5\n");
  sprintf(result,"%s],\"local_millisecond\":\"%llu\"",result,_sD.time);  
  sprintf(result,"%s,\"host_name\":\"%s\"",result,_sD.hostname);    
  sprintf(result,"%s,\"host_ip\":\"%i.%i.%i.%i\"",result,_sD.ip[0],_sD.ip[1],_sD.ip[2],_sD.ip[3]);      
  sprintf(result,"%s}",result);  
  free(mmB); 
}


void *update_fresh_data(void* data){
  while(1){    
    memBlock *mmB = calloc(DMEMORY,1);
      if(_data_list!=NULL){        
        memcpy(mmB,_data_list,DMEMORY);        
        while (mmB != NULL) {       
          if(mmB->fresh_time >= 30){
            delMemBlock(_data_list,mmB);
          }else{                      
            memset((mmB->fresh_time),(mmB->fresh_time+1),0);                
          }
          mmB = mmB->next;
      }
      free(mmB);
    }
    usleep(1000);
  }
  return NULL; 
}

void* udpServerThread(void *port)
{    
  //sem_init(&mutex, 0, 1);
  int sockfd;
  struct sockaddr_in si_me, si_other;
  char buffer[1024]={0};
  socklen_t addr_size;
  memBlock *_data = calloc(sizeof(memBlock), 1);
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&si_me, '\0', sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = inet_addr("0.0.0.0");
  
  bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
  addr_size = sizeof(si_other);  
  int err = pthread_create(&tid_refresh, NULL, update_fresh_data,(void*)&port);  

  while(1){    
    printf("udpServerThread pre recvfrom\n");   
    
  	recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&si_other, &addr_size);     
    
    printf("udpServerThread pre memcpy\n");       
    memcpy(_data,buffer,DMEMORY); 	  
    printf("udpServerThread after memcpy\n");       
    /*
    printf("recev| ip_received  %i.%i.%i.%i  time %llu\n",
            (u_int8_t)_data.ip_received[0],(u_int8_t)_data.ip_received[1],(u_int8_t)_data.ip_received[2],(u_int8_t)_data.ip_received[3],            
            _data.time_);    
    */
    //sprintf(ip_socket,"%s",inet_ntoa(si_other.sin_addr));    

    _data->validate_data = 1;
    printf("udpServerThread pre storedata\n");   
    //printBufferFull(struct_size);
    addMemBlock(&_data_list,_data);
    //storeData(&_data);    
    //sem_post(&mutex); 
    //printBufferFull(struct_size);    
    printf("udpServerThread after storeData #2\n");   
    memset(&buffer,0,1024);                      
    printf("udpServerThread after memset #2\n");   
  }
  return NULL;
}

int udpServer(int port)
{       
  //sem_init(&mutex, 0, 1);   
  int err = pthread_create(&tid, NULL, &udpServerThread,port);  
  if (err != 0){ 
    printf("\ncan't create thread :[%s]", strerror(err));
    return 0;
  }else{
    printf("\n Thread created successfully\n");    
  }
  return 1;
}