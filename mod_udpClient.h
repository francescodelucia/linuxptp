#ifndef __MOD_UDPCLIENT_H_
#define __MOD_UDPCLIENT_H_
#ifdef  __cplusplus
extern "C" {
#endif
extern void* udpServerThread(void *arg);
extern int udpServer(int port);

#ifdef  __cplusplus
}
#endif
#endif