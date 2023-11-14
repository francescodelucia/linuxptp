#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mod_memManager.h"


memBlock* newMemBlock(memBlock* data) {

    memBlock* _new = malloc(DMEMORY);
    memcpy(_new, data, DMEMORY);   
    _new->next = NULL;
    return _new;
}

void addMemBlock(memBlock** _list, memBlock* data) {    
     
     memBlock *p_list = *_list;
    while (p_list != NULL) {
        if (*((uint32_t*)&p_list->ip_received) == *((uint32_t*)&data->ip_received)) {
            *((uint32_t*)&p_list->time_) = data->time_;
            return;
        }
        p_list = p_list->next;
    }
    memBlock* _new = malloc(DMEMORY);    
    memcpy(_new, newMemBlock(data), DMEMORY);    
    if (*_list == NULL) {
        *_list = _new;
    }
    else {
        memBlock* _last = *_list;
        while (_last->next != NULL) {
            _last = _last->next;
        }
        _last->next = _new;
    }
}


void delMemBlock(memBlock** _list, memBlock* data) {
    memBlock* elem = *_list;
    while (elem != NULL && (((uint32_t) * (elem->ip_received)) != ((uint32_t) * (data->ip_received)))) {
        elem = elem->next;
    }
    if (elem == NULL) {
        printf("Elemento non trovato\n");
        return;
    }
    if (elem == *_list) {
        *_list = elem->next;
    }
    else {
        memBlock* prev = *_list;
        while (prev->next != elem) {
            prev = prev->next;
        }
        prev->next = elem->next;
    }
    free(elem);
}

memBlock* searchElem(memBlock* _list, memBlock* data) {    
    while (_list != NULL) {
        if (*((uint32_t*)&_list->ip_received) == *((uint32_t*)&data->ip_received)){            
            return _list;
        }        
        _list =  _list->next;        
    }
    return NULL;
}

/*
void  addElemfromIP(memBlock* _list, memBlock* data) {    
    int isPresent = 0;
    while (_list != NULL) {
        if (*((uint32_t*)&_list->ip_received) == *((uint32_t*)&data->ip_received)){    
            _list->time_ = data->time_;        
            isPresent = 1;
        }        
        _list =  _list->next;        
    }
    if(isPresent==0){
        addMemBlock(&_list,data);
    }    
}
*/
