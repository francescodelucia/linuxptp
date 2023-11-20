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
#include <time.h>
#include "mod_memManager.h"
#include "mod_udpServer.h"

memBlock *_data_list;
serverData _sData;

int struct_size=0;
pthread_t tid;
pthread_t tid_refresh;

void set_Server_Config(void *m_c){  
  memcpy(&m_config,m_c,sizeof(struct mod_config));
}

void getHostNameInfo(char *out){
  char host_name[256]={0};
  struct hostent *host_entry;
  int ihost;
  ihost = gethostname(host_name,256);
  host_entry = gethostbyname(host_name);  
  sprintf(out,"%s",host_name);
}

void updateServerData(serverData *sD){  
  /*char host_name[256]={0};
  struct hostent *host_entry;
  int ihost;
  ihost = gethostname(host_name,256);
  host_entry = gethostbyname(host_name);  
  sprintf(sD->hostname,"%s",host_name);*/
  getHostNameInfo(sD->hostname);
  sD->time = timeInMilliseconds();      
  char r[4];
  getIP(&r);          
  *((uint32_t*)&sD->ip) = *((uint32_t*)&r);      
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
  sprintf(ret,"%i.%i.%i.%i",ret,ip[0],ip[1],ip[2],ip[3]); 
  return &ret;
}

void printStringDebug(char *str){    
  if(str!=NULL){
    printf("printStringDebug\n");
    for(int k=0;k<strlen(str);k++){             
      printf("%i ",str[k]);    
    }
    printf("\n");                    
    printf("%s\n",str);
  }
} 

void printMemDebug(char *str){  
  if(str!=NULL){
    printf("printMemDebug\n");
    for(int k=0;k<DMEMORY;k++){            
      printf("%i ",(uint8_t)str[k]);    
    }
    printf("\n"); 
  }else{
    printf("printMemDebug - data not avalible\n");
  }                     
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
  strncpy(ifr.ifr_name, m_config.iface, IFNAMSIZ-1);
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

int getMillisecondsToTimeStamp(long long msecs,char *out,int size_out){   
  time_t seconds =  msecs / 1000; // Convert milliseconds to seconds
  struct tm *tm = localtime(&seconds); // Convert seconds to a struct tm using localtime
  if (tm == NULL) { // Check if localtime conversion failed
    perror("localtime"); // Print error message if conversion failed
    return 1;
  }  
  strftime(out, size_out, "%Y-%m-%d %H:%M:%S", tm); // Format struct tm to a date and time string with milliseconds
  return 0;
}

void get_live_data(char *result ) {        
  sprintf(result,"{\"data\":[");  
  serverData _sD;
  updateServerData(&_sD);
  memBlock *mmB=calloc(sizeof(memBlock*),1);   
  char out[80]={0};   
  mmB = _data_list;          
  while (mmB != NULL) {     
    //long long t=mmB->time_;  
    getMillisecondsToTimeStamp(mmB->time_,&out,80);               
    sprintf(result,
      "%s{\"HOSTNAME\":\"%s\",\"IP_CLIENT\":\"%i.%i.%i.%i\",\"TIME\":\"%s\",\"DIFF_SERVER ms\":\"%llu\",\"FRESH_DATA\":\"%i\",\"RES_1\":\"--\"},",
      result,
      mmB->hostname,
      (u_int8_t)mmB->ip_received[0],
      (u_int8_t)mmB->ip_received[1],
      (u_int8_t)mmB->ip_received[2],
      (u_int8_t)mmB->ip_received[3],          
      out,
      abs(mmB->time_-_sD.time),
      mmB->fresh_time
    );              
    mmB = mmB->next;
    if(mmB == NULL){
      break;
    }    
  }       
  if(result[strlen(result)-1]==','){
    result[strlen(result)-1]=0;  
  }      
  memset(out,0,80);
  getMillisecondsToTimeStamp(_sD.time,&out,80);   
  
  sprintf(result,"%s],\"local_millisecond\":\"%s\"",result,out);  
  sprintf(result,"%s,\"host_name\":\"%s\"",result,_sD.hostname);    
  sprintf(result,"%s,\"host_ip\":\"%i.%i.%i.%i\"",result,_sD.ip[0],_sD.ip[1],_sD.ip[2],_sD.ip[3]);      
  sprintf(result,"%s,\"RESER_1\":\"--\"",result);      
  sprintf(result,"%s,\"RESER_2\":\"--\"",result);      
  sprintf(result,"%s,\"RESER_3\":\"--\"",result);      
  sprintf(result,"%s}",result);  
  free(mmB); 
}


void *update_fresh_data(void* data){
  while(1){        
    memBlock *mmB=calloc(sizeof(memBlock*),1);      
    mmB = _data_list;                
    while (mmB != NULL) {       
      if(mmB->fresh_time >= 30){
        delMemBlock(&_data_list,mmB);
      }else{                             
        mmB->fresh_time=mmB->fresh_time+1;                
      }
      mmB = mmB->next;
      if(mmB == NULL){
        break;
      }  
    }
    free(mmB);    
    sleep(1);
  }
  return NULL; 
}

void* udpServerThread(void *port)
{     
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
  	recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&si_other, &addr_size);        
    memcpy(_data,buffer,DMEMORY); 	  
    _data->validate_data = 1;        
    addMemBlock(&_data_list,_data);    
    memset(&buffer,0,1024);                          
  }
  return NULL;
}

int udpServer(int port)
{         
  int err = pthread_create(&tid, NULL, &udpServerThread,port);  
  if (err != 0){ 
    printf("\ncan't create thread :[%s]", strerror(err));
    return 0;
    //}else{
    //printf("\n Thread created successfully\n");    
  }
  return 1;
}