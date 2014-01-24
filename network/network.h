#ifndef _NETWORK_H_
#define _NETWORK_H_


#include <stdint.h>
#include <string.h>
#include "../nrf24l01/nrf24l01.h"
#include "movement/movement.h"
#define _MAX_PIPES_ 5
#define INVALID_GID 0

#define DEFAULT_PIPE 0
#define DEFAULT_TTL 7

typedef struct
{
    uint16_t id;
    uint8_t pipe;        
}
ripEntry_t;
typedef ripEntry_t * ripEntry_p;

/* Possible packet types */
typedef enum {
    INVALID,                          
    DATA, /* The message is for you */ 
    DISCOVERY, /* I want to join the network */  
    BROADCAST /* Read & re-transmit the message */
}
packet_t;

typedef struct {
    uint8_t     type;
    uint8_t     size;
    uint8_t     checksum;
    uint8_t     ttl;
    uint16_t    idSrc;
    uint16_t    idDest;    
}
headerPack_t;
typedef headerPack_t * headerPack_p;

typedef struct {
    headerPack_t header;
    uint8_t data[8];    
}
discPack_t;
typedef discPack_t * discPack_p;

extern ripEntry_t ripTable[_MAX_PIPES_];
extern uint8_t usedEntries;
extern uint16_t gID;    

/* Internal, aux functions */
int isRootPipe(uint16_t pipe);
int isInRange(uint16_t leafPipe, uint16_t rootPipe);
ret_t insertEntry(ripEntry_t *newEntry);
uint8_t checksumCalculator(headerPack_p hdr, 
                                   char *msg, 
                                uint8_t left);
 
 
/* ------ To use outside this API ------  */
 ret_t joinNetwork();
//Broadcast message
 ret_t sendMessage(char *msg,  uint8_t size);
//Send message to an specific ID
ret_t sendMessageTo(uint16_t id, packet_t type, 
                        char *msg, uint8_t size);
//Recieve message from any sender
 ret_t getMessage(char *buf, uint16_t size);
//Specify ID to recieve from
 ret_t getMessageFrom(uint16_t id, char *buf, uint16_t size); 

 
#endif 
