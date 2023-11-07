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

void* udpClientThread(void *port)
{    

  memData md;
  
  char *ip = "192.168.0.103";   
  mod_parse_ip(ip,strlen(ip),md.ip_aspected);
  mod_parse_ip(ip,strlen(ip),md.ip_received);
 
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  char buff[100];
  strftime(md.time_, sizeof buff, "%D %T", gmtime(&ts.tv_sec));

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;
 
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
 
  bzero(buffer, 1024);
  memcpy(&buffer,&md,sizeof(memData));    
  sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));  

  return 0;
}

int udpClient(int port)
{       
  int err = pthread_create(&tidClient, NULL, &udpClientThread,8090);  
  if (err != 0){     
    return 0;
  }  
  return 1;
}