#pragma once
#ifndef __MOD_UDP_SERVER_H__
#define __MOD_UDP_SERVER_H__
#ifdef  __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include "mod_memManager.h"


#define CONTENT_TYPE_TEXT 0
#define CONTENT_TYPE_JSON 1
#define MAX_SIZE    512
#define IS_JSON     1
#define NOT_JSON    0
#define MODE_CLIENT 0x0
#define MODE_SERVER 0x1
#define NO_MODE     -1

#define NO_DATA_FOUND -1
#define IS_NOT_VOID 0xff
#define IS_VOID 0


#define UINT8_MAX 0xff
#define ERROR_IPV4_DATA_OVERFLOW    0xf1
#define ERROR_IPV4_NO_SYMBOL        0xf2    
#define ERROR_IPV4_INPUT_OVERFLOW   0xf3
#define ERROR_IPV4_INVALID_SYMBOL   0xf4
#define ERROR_IPV4_NOT_ENOUGH_INPUT 0xf5


typedef struct {
  unsigned char hostname[50];
  long long time;
  unsigned char ip[4];
} serverData;
 
typedef struct {  
  unsigned char ip[4];
  unsigned char _update;
  unsigned char remoteIp[20];
} localInfo;
 
static struct mod_config {
  int port_www;
  int udp_port;
  int mode;
  int remoteIP [4];
};

extern long long timeInMilliseconds(void);
extern int mod_parse_ip(const uint8_t * string,uint8_t string_length,int* result);
extern  struct mod_config get_base_mod_config();
extern void printBuffer(int size);
extern void printStringDebug(char *str);
extern void replace_char_from_string(char from, char to, char *str);
extern void parseData(char* buff,void *_data);
extern memBlock *findIp(void *_data);
//extern int memVal(void *_data,int size);
extern void *update_fresh_data();
extern void* udpServerThread(void *arg);
extern int udpServer(int port);
extern const char * get_ip();
extern void getIP(unsigned char *out );


#ifdef  __cplusplus
}
#endif

#endif //__MOD_UDP_SERVER__