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

localInfo _lInfo;


void set_Config(void *m_c){  
  memcpy(&m_config,m_c,sizeof(struct mod_config));
}

void* udpClientThread(void *param)
{    
  memBlock md;   
  char ip[15] = {0};
  char tim_[20]={0};  
  char lip[15] = {0};
  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];    

  while(1){    
    if(_lInfo._update == 1){
      if((_lInfo.ip[0]+_lInfo.ip[1]+ _lInfo.ip[2]+ _lInfo.ip[3])==0){
        unsigned char IP[4]={0};
        getIP(IP);  
        memcpy(_lInfo.ip,IP,4);      
      }
      sprintf(ip,"%s",_lInfo.remoteIp); 
      md.isUsed = IS_NOT_VOID; 
      memcpy(md.ip_received,_lInfo.ip,4);    
      md.time_= timeInMilliseconds();  
      socklen_t addr_size;  
      sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      memset(&addr, '\0', sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(m_config.udp_port);
      addr.sin_addr.s_addr = inet_addr(ip);   
      bzero(buffer, 1024);
      memcpy(&buffer,&md,sizeof(memBlock));      
      for(int k=0;k<(sizeof(memBlock));k++){    
          printf("%i ",(unsigned char)buffer[k]);            
      }
      /*printf("\n");                
      printf("%llu\n",md.time_);  */  
      sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));  
      _lInfo._update = 0;
    }
    sleep(1);
  }
  return 0;
}

void updateData(void *param){
  sprintf(_lInfo.remoteIp,"%s",param);
  _lInfo._update = 1;
}

int udpClient(void *param)
{           
  int err = pthread_create(&tidClient, NULL, &udpClientThread,param);  
  if (err != 0){     
      return 0;
  }    
  return 1;
}