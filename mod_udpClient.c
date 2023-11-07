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

void ip_to_int (const char * ip,unsigned char* out)
{
    unsigned char ipstr[15] = {0};
    char *marker, *ret;
    unsigned char ipout[4];
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
}

void* udpClientThread(void *remoteIP)
{    

  memData md;
  
  char ip[15] = {0};
  sprintf(ip,"%s",remoteIP);
  char tim_[20]={0};

  char lip[15] = {0};
  sprintf(lip,"%s",get_ip());
  ip_to_int(lip,md.ip_aspected);
  ip_to_int(lip,md.ip_received);  
 

  sprintf(tim_,"%llu",timeInMilliseconds());

  memcpy(md.time_,tim_,strlen(tim_));

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;
 
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_config.udp_port);
  addr.sin_addr.s_addr = inet_addr(ip); 
 
  bzero(buffer, 1024);
  memcpy(&buffer,&md,sizeof(memData));      
  for(int k=0;k<(sizeof(memData));k++){    
      printf("%i ",(unsigned char)buffer[k]);            
  }
  printf("\n");                
  printf("%s\n",buffer);
  printf("%s\n",md.time_);
  printf("UDP %i\n",m_config.udp_port);

  sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));  

  return 0;
}

int udpClient(void *param)
{       

  printf("----------\n");  
  int err = pthread_create(&tidClient, NULL, &udpClientThread,param);  
  if (err != 0){     
    return 0;
  }  
  return 1;
}