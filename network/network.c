#include "network.h"
#include <stdint.h>

ripEntry_t ripTable[_MAX_PIPES_] = {{0,0},
                                    {0,0},
                                    {0,0},
                                    {0,0},
                                    {0,0}};
uint8_t usedEntries = 0;
uint16_t gID = 0;  

#include "platform.h"
#include "stdlib.h"
#include <delay.h>

#define _JOIN_PIPE_ 0
#define _MAX_RETRIES_ 100
#define _DELTA_DELAY_ 10
// Voltage Reference: AREF pin
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))


//To use outside this API
ret_t joinNetwork()
{
    // ----------- Variable declarations ----------
    uint8_t pipe;   
    char data[NRF24L01_PAYLOAD];   
    ripEntry_t *entryP = (ripEntry_t *)data;    
    uint8_t retryN = 0;
    discPack_t packet;
    headerPack_p hdr = (headerPack_p)&packet;
                                                   
    // --------------------------------------------
    srand(TCNT2);

    gID = rand(); 
    printf("gID %s:%d:%d\n", __FILE__, __LINE__, gID);   
    
    // Select initial pipe, range 1-255
    pipe = gID%255 + 1;
    // Validate root
    if(isRootPipe(pipe))
    {  
        while(retryN < _MAX_RETRIES_)
        {
            //Wait for a non-root node in range to send a message
            while( !nrf24l01_readready(_JOIN_PIPE_) );
            //Someone sends us a message  
            nrf24l01_read(data);
            if(isInRange(entryP->pipe, pipe)) 
            {
                if(usedEntries<_MAX_PIPES_)
                {
                    insertEntry(entryP);
                }  
                else                           
                {
                    //Convert leaf node to root 
                    //break outter while
                }
            }
            else         
            {
                retryN++;
            }
        }                              
    }
    else // Leaf node
    {
        //Look for our root to become available
        while(retryN < _MAX_RETRIES_)
        {
            //Check if someone has just sent something
            if( !nrf24l01_readready(_JOIN_PIPE_) )
            {
                //Send a message to root, build packet to send
                hdr->checksum = checksumCalculator(hdr,data,0); //xor with 0 unaffects the checksum
                hdr->idDest = ;
                hdr->idSrc = gID;
                hdr->size = size;
                hdr->ttl = DEFAULT_TTL;
                hdr->type = type;
            }
            if(isInRange(entryP->pipe, pipe)) 
            {
                if(usedEntries<_MAX_PIPES_)
                {
                    insertEntry(entryP);
                }  
                else                           
                {
                    //Convert leaf node to root 
                    //break outter while
                }
            }
            else         
            {
                retryN++;
            }
        }      
    }                                                             
    return SUCCESS;
}
//Broadcast message originated in this host
ret_t sendMessage(char *msg,  uint8_t size)
{
  uint8_t idx;
  ripEntry_p re;         

  for(idx=0; idx<_MAX_PIPES_; idx++)
  {
    re = &ripTable[idx];
    if(re->id == INVALID_GID)
      continue;
    sendMessageTo(re->id, BROADCAST, msg, size);    
  }
    return SUCCESS;
}
//Send message to an specific ID
ret_t sendMessageTo(uint16_t id, packet_t type, 
                        char *msg, uint8_t size)
{
  uint8_t idx, pipe = DEFAULT_PIPE, done = 0, found = 0;
  uint8_t checksum;
  ripEntry_p re; 
  discPack_t packet;
  headerPack_p hdr = (headerPack_p)&packet;

  hdr->type = type;
  hdr->size = size;
  hdr->ttl = DEFAULT_TTL;
  hdr->idSrc = gID;
  hdr->idDest = id;

  for(idx=0; idx<_MAX_PIPES_; idx++)
  {                          
    re = &ripTable[idx];
    if(re->id == INVALID_GID)
      continue;
    if(re->id == id)
    {
      pipe = re->pipe;                                 
      found = 1;
      break;
    }
  }
  idx = 0;
nextEntry:
  if(!found)
  {
    if(idx == _MAX_PIPES_)
      return WARNING;
    re = &ripTable[idx];    
    if(re->id == INVALID_GID)
    {
      idx++;
      goto nextEntry;
    }
    pipe = re->pipe;
  }
  
  while(done <= size)
  {
    hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
    memcpy(packet.data, &msg[done], 8);
    nrf24l01_settxaddr(nfr23l01_pipeAddr(nrf24l01_addr, pipe));
    nrf24l01_write((uint8_t *)&packet);
    done += 8;
  }
  idx++;
  if(!found)
    goto nextEntry;
  return SUCCESS;
}
//Recieve message from any sender
ret_t getMessage(char *buf, uint16_t size){
    return UNIMPLEMENTED;
}
//Specify ID to recieve from
ret_t getMessageFrom(uint16_t id, char *buf, uint16_t size){
    return UNIMPLEMENTED;
}

ret_t insertEntry(ripEntry_t *newEntry)
{
    return UNIMPLEMENTED;
}

/* Internal, aux functions */
int isRootPipe(uint16_t pipe)
{
    return 0;
}

int isInRange(uint16_t leafPipe, uint16_t rootPipe)
{
    return 0;    
}

/*
 * param left: Size of bytes left to send 
 */

uint8_t checksumCalculator(headerPack_p hdr, 
                                   char *msg, 
                                uint8_t left)
{
  uint8_t i, checksum = 0;
  uint8_t limit = left < 8 ? left : 8;
  checksum ^= hdr->type;
  checksum ^= hdr->size;
  checksum ^= hdr->ttl;
  checksum ^= (uint8_t)((hdr->idSrc & 0xFF00)>>8);
  checksum ^= (uint8_t)(hdr->idSrc & 0x00FF);
  checksum ^= (uint8_t)((hdr->idDest & 0xFF00)>>8);
  checksum ^= (uint8_t)(hdr->idDest & 0x00FF);
  
  for(i = 0; i < limit; i++)
    checksum ^= msg[i];
    
  return checksum;
}