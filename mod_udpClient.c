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
#include <netinet/in.h>
//#include "mod_const.h"
#include "mod_udpClient.h"
#include "mod_udpServer.h"


pthread_t tidClient;

static struct mod_config m_config;


void set_Config(void *m_c){  
  memcpy(&m_config,m_c,sizeof(struct mod_config));
}

void ip_to_int (const unsigned char * ip,unsigned char* out)
{

    unsigned char ipstr[15] = {0};
    char *marker, *ret;
    unsigned char ipout[4];
    if(ip!=NULL){
      printf("ip_to_int - ##1\n");
      memcpy(ipstr,ip,sizeof(unsigned char)*15);
      ret = strtok_r(ipstr, ".", &marker);
      printf("ip_to_int - ##2\n");
      ipout[0] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);
      printf("ip_to_int - ##3\n");
      ipout[1] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);
      printf("ip_to_int - ##4\n");
      ipout[2] = (char)strtod(ret, NULL);
      ret = strtok_r(NULL, ".", &marker);
      printf("ip_to_int - ##5\n");
      ipout[3] = (char)strtod(ret, NULL);        
      printf("ip_to_int - ##6\n");
      memcpy(out,&ipout,sizeof(unsigned char)*4);
      printf("ip_to_int - ##7\n");
    }else{
      memset(out,0,4);
    }
}

void* udpClientThread(void *remoteIP)
{    

  memData md;
  printf("##1");
  char ip[15] = {0};
  sprintf(ip,"%s",remoteIP);
  char tim_[20]={0};
  printf("##2");
  char lip[15] = {0};
  sprintf(lip,"%s",get_ip());
  printf("##2-0 %s",lip);
  //ip_to_int(lip,md.ip_aspected);
  //printf("##2-1 %s",lip);
  ip_to_int(lip,md.ip_received);  
  printf("##3");

  md.time_= timeInMilliseconds();

  //memcpy(md.time_,tim_,strlen(tim_));

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;
  printf("##4");
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_config.udp_port);
  addr.sin_addr.s_addr = inet_addr(ip); 
  printf("##5");
  bzero(buffer, 1024);
  memcpy(&buffer,&md,sizeof(memData));      
  for(int k=0;k<(sizeof(memData));k++){    
      printf("%i ",(unsigned char)buffer[k]);            
  }
  printf("\n");                
  //printf("%s\n",buffer);
  printf("%llu\n",md.time_);  
  printf("##6");
  sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));  

  return 0;
}

int udpClient(void *param)
{       
  int err = pthread_create(&tidClient, NULL, &udpClientThread,param);  
  if (err != 0){     
    return 0;
  }  
  return 1;
}