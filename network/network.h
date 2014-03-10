#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <math.h>
#include <delay.h>
#include <stdint.h>
#include <string.h>

#include "../nrf24l01/nrf24l01.h"
#include "../movement/movement.h"
#include "../random/random.h"
#include "../platform.h"

#define _MAX_PIPES_ 5
#define _MAXIMUM_ADDRESS_ 255
#define INVALID_GID 0

#define DEFAULT_PIPE 0
#define DEFAULT_TTL 7
#define DEFAULT_TIMEOUT 100000
#define DEFAULT_DELAY 100


#define _JOIN_PIPE_ 0
#define _MAX_RETRIES_ 100
#define _DELTA_DELAY_ 10

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
    RAW,
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

/* ------ To use outside this API ------  */
ret_t networkManager(headerPack_p header,
              char *buffer, uint8_t size);
ret_t joinNetworkOnTheFly(discPack_p pp);
//Forward a broadcast message
ret_t forwardMessage(headerPack_p header,
                     char *msg,  uint8_t size);
//Broadcast message
ret_t sendMessage(char *msg,  uint8_t size);
/*
 * Advanced way to send a message.
 * No verification of any type is done.
 * header + msg are transmited in one packet.
 */
ret_t microSendMessage(headerPack_p header,
                     char *msg,  uint8_t size);
//Send message to an specific ID
ret_t sendMessageTo(headerPack_p header,
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

/* Internal, aux functions */
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


#define INIT_NW_STACK()  nrf24l01_init()

#define GET_ADDRESS(a) \
  ((a)%_MAXIMUM_ADDRESS_ + 1)

#define isRootId(_id) \
    (((_id)%((uint16_t)(_MAX_PIPES_))) == 0)

#define isInRange(_leafID, _rootID) \
    (getRootFromID(_leafID) == (_rootID))

#define getRootFromID(_id) \
    _MAX_PIPES_*floor((float)(_id)/_MAX_PIPES_)

#define printHeader(hdr) \
//    printf("Package header\nhdr addr:%d\ntype:%d\nsize:%d\nchecksum:%d\nttl:%d\nnumber:%d\nsrcID:%d\ndestID:%d\n\n", \
//    (hdr), (hdr)->type, (hdr)->size, (hdr)->checksum, (hdr)->ttl, (hdr)->number, (hdr)->idSrc, (hdr)->idDest)

#define printRipEntry(re) \
//    printf("RipEntry\nrip entry addr:%d\naddress:%d\nisRoot:%d\nid:%d\n\n", \
//           (re), (re)->address, (re)->isRoot, (re)->id)

#define printRootReply(rr) \
//    printf("RootRepy addr: %d\nisAcceptedConnection:%d\naddress:%d\ngID2:%d\n", \
//          (rr), (rr)->isAcceptedConnection, (rr)->address, (rr)->gID2)

#define printRipTable() \
//    do{ \
//        uint8_t __i; \
//        printf("ripTable contents\n"); \
//        for(__i=0; __i<_MAX_PIPES_; __i++) \
//            printRipEntry(&(ripTable[__i])); \
//    }while(0)

#define netDebugPrint printf

#endif
