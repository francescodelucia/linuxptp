#ifndef __MOD_MEM_MANAGER_H__
#define __MOD_MEM_MANAGER_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define DMEMORY sizeof(memBlock)

#define MEM_DATA_CPY  memcpy(_new->nome,data.nome,strlen(data.nome));\
                      memcpy(_new->ip_received,data.ip_received,4);\
                      memcpy(_new->time_,data.time_,sizeof(long long));\
                      memcpy(_new->validate_data,data.validate_data,sizeof(unsigned char));\
                      memcpy(_new->fresh_time,data.fresh_time,sizeof(unsigned char));\
                      memcpy(_new->_update,data._update,sizeof(unsigned char));                     

    /*
    #define MEM_DATA_CPY memcpy(_new->nome,nome,strlen(nome));\
                        memcpy(_new->ip_received,ip_received,4);\
                        memcpy(_new->time_,time_,sizeof(long long));\
                        memcpy(_new->validate_data,validate_data,sizeof(unsigned char));\
                        memcpy(_new->fresh_time,fresh_time,sizeof(unsigned char));\
                        memcpy(_new->_update,_update,sizeof(unsigned char));
    */
    typedef struct {
        unsigned char isUsed;
        unsigned char ip_received[4];
        unsigned char hostname[40];
        long long time_;
        unsigned char validate_data;
        unsigned char fresh_time;
        unsigned char _update;
        char nome[30];
        void* next;
    } memBlock;

    extern void delMemBlock(memBlock** _list, memBlock* data);
    extern memBlock* newMemBlock(memBlock* data);
    extern void addMemBlock(memBlock** _list, memBlock* data);
    extern memBlock* searchElem(memBlock* _list, memBlock* data);
    //extern void  addElemfromIP(memBlock* _list, memBlock* data);   


#ifdef  __cplusplus
}
#endif

#endif