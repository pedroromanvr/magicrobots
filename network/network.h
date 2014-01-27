#ifndef _NETWORK_H_
#define _NETWORK_H_


#include <stdint.h>
#include <string.h>
#include "../nrf24l01/nrf24l01.h"
#include "movement/movement.h"
#define _MAX_PIPES_ 5
#define _MAXIMUM_ADDRESS_ 255
#define INVALID_GID 0

#define DEFAULT_PIPE 0
#define DEFAULT_TTL 7
#define TRUE    1
#define FALSE   0

#define DATA_SIZE \
  (NRF24L01_PAYLOAD-sizeof(headerPack_t))

typedef struct
{
    uint8_t address; 
    uint8_t isRoot; 
    uint16_t id;           
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
    uint8_t     number;
    uint16_t    idSrc;
    uint16_t    idDest;    
}
headerPack_t;
typedef headerPack_t * headerPack_p;

typedef struct {
    uint8_t     isAcceptedConnection; /* Boolean true if connection was accepted */
    uint8_t     address;   /* Address to be paired with */
    uint16_t    gID2;   /* Alternative global ID(root) to pair with */ 
}
rootReplyP_t;
typedef rootReplyP_t * rootReplyP_p;

typedef struct {
    headerPack_t header;
    uint8_t data[DATA_SIZE];    
}
discPack_t;
typedef discPack_t * discPack_p;

extern ripEntry_t ripTable[_MAX_PIPES_];
extern uint8_t usedEntries;
extern uint8_t isPaired;
extern uint8_t isRoot;
extern uint16_t gID;    

/* Internal, aux functions */
int isRootId(uint16_t id);
int isInRange(uint16_t leafID, uint16_t rootID);
uint16_t getRootFromID(uint16_t address);
uint8_t getFreeAddress(uint8_t rootAddr);
uint8_t isInRIP(uint8_t inpAddr);

ret_t insertEntry(ripEntry_p newEntry);
uint8_t checksumCalculator(headerPack_p hdr, 
                                   char *msg, 
                                uint8_t left);
/*
 * pipe : IN
 * addr : OUT (5 byte array)
*/
ret_t getAddrByPipe(uint8_t pipe, char *addr);
 
 
/* ------ To use outside this API ------  */
 ret_t joinNetwork();
//Broadcast message
 ret_t sendMessage(char *msg,  uint8_t size);
//Send message to an specific ID
ret_t sendMessageTo(uint16_t id, packet_t type, 
                        char *msg, uint8_t size);
/*
 * header is used to retrieve information
 * about the message.
 * In getMessageFrom it is also used to indicate
 * the source ID
 */
//Recieve message from any sender
 ret_t getMessage(headerPack_p header, 
                 char *buf, uint8_t size);
//Specify ID to recieve from
 ret_t getMessageFrom(headerPack_p header, 
                 char *buf, uint8_t size); 

 
#endif 
