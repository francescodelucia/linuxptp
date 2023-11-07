#pragma once
#ifndef __MOD_UDP_SERVER__
  #define __MOD_UDP_SERVER__
#include <unistd.h>
#ifdef  __cplusplus
extern "C" {
#endif



#define CONTENT_TYPE_TEXT 0
#define CONTENT_TYPE_JSON 1
#define MAX_SIZE    512
#define IS_JSON     1
#define NOT_JSON    0
#define MODE_CLIENT 0x0
#define MODE_SERVER 0x1
#define NO_MODE     -1

#define UINT8_MAX 0xff
#define ERROR_IPV4_DATA_OVERFLOW    0xf1
#define ERROR_IPV4_NO_SYMBOL        0xf2    
#define ERROR_IPV4_INPUT_OVERFLOW   0xf3
#define ERROR_IPV4_INVALID_SYMBOL   0xf4
#define ERROR_IPV4_NOT_ENOUGH_INPUT 0xf5

static struct mod_config {
  int port_www;
  int udp_port;
  int mode;
  int remoteIP [4];
};


typedef struct  {
  char ip_aspected[4];
  char ip_received[4];
  char time_[50];
  uint8_t validate_data;
  int fresh_time;  
  uint8_t _update;
} memData;

extern int mod_parse_ip(const uint8_t * string,uint8_t string_length,int* result);
extern  struct mod_config get_base_mod_config(g);
extern void printBuffer(int *size);
extern void printStringDebug(char *str);
extern void replace_char_from_string(char from, char to, char *str);
extern void parseData(char* buff,void *_data);
extern int findIp(void *_data,int *struct_size);
extern int memVal(void *_data,int size);
extern void *update_fresh_data();
extern void* udpServerThread(void *arg);
extern int udpServer(int port);

#ifdef  __cplusplus
}
#endif

#endif