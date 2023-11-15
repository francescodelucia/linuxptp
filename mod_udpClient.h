#ifndef __MOD_UDPCLIENT_H_
#define __MOD_UDPCLIENT_H_
#ifdef  __cplusplus
extern "C" {
#endif



extern void set_Config(void *);
extern void* udpServerThread(void *);
extern int udpServer(int);

#ifdef  __cplusplus
}
#endif
#endif