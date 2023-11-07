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

void ip_to_int (const char * ipstr,int* out)
{
    //char ipstr[] = "121.122.123.124";
    char *marker, *ret;
    //unsigned char b1, b2, b3, b4;
    ret = strtok_r(ipstr, ".", &marker);
    out[0] = (unsigned char)strtod(ret, NULL);
    ret = strtok_r(NULL, ".", &marker);
    out[1] = (unsigned char)strtod(ret, NULL);
    ret = strtok_r(NULL, ".", &marker);
    out[2] = (unsigned char)strtod(ret, NULL);
    ret = strtok_r(NULL, ".", &marker);
    out[3] = (unsigned char)strtod(ret, NULL);    
}


void* udpClientThread(void *port)
{    

  memData md;
  
  char *ip = "192.168.0.106";   
  //mod_parse_ip(ip,strlen(ip),&md.ip_aspected);
  //mod_parse_ip(ip,strlen(ip),&md.ip_received);
  ip_to_int(ip,&(md.ip_aspected));
  ip_to_int(ip,&(md.ip_received));
  printf("%s\n",md.ip_aspected);
  printf("%s\n",md.ip_aspected);
 
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
   printf("----------\n");    
  for(int k=0;k<(512);k++){    
      printf("%i ",buffer[k]);            
  }
  printf("\n");                
  printf("%s\n",buffer);

  sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));  

  return 0;
}

int udpClient(int port)
{       
  printf("----------\n");  
  int err = pthread_create(&tidClient, NULL, &udpClientThread,8090);  
  if (err != 0){     
    return 0;
  }  
  return 1;
}