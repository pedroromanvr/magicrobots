#include "network.h"
#include <stdint.h>

ripEntry_t ripTable[_MAX_PIPES_] = {{0,0,0},
                                    {0,0,0},
                                    {0,0,0},
                                    {0,0,0},
                                    {0,0,0}};
uint8_t usedEntries = 0;
uint8_t isPaired = FALSE;
uint8_t isRoot = 255;   //Initialize with any value
uint16_t gID = 0;  

#include "../movement/movement.h"
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
    uint8_t retryN = 0;
    
    //Used by root node
    ripEntry_p entryP = (ripEntry_p)data;    
    
    
    //Used by leaf node
    discPack_t packet;
    headerPack_p hdr = (headerPack_p)&packet;
    rootReplyP_p rootReply = (rootReplyP_p)data;
    ripEntry_t myLeafInfo;
                                                   
    // --------------------------------------------
    srand(TCNT2);

    gID = rand(); 
    printf("gID %s:%d:%d\n", __FILE__, __LINE__, gID);   
    
    // Select initial pipe, range 1-255
    pipe = gID%255 + 1;
    // Validate root
    if(isRootId(gID))
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
                //Send a message to our root, build packet to send
                myLeafInfo.id   = gID;
                myLeafInfo.pipe = pipe;
                myLeafInfo.isRoot = 0;
                
                sendMessageTo(getRootFromRange(pipe), DISCOVERY,
                              (char *)&myLeafInfo, sizeof(myLeafInfo));
            }
            else
            {
                //Verify packet destiny
                nrf24l01_read(data);
                if(isRootId(rootID))
                {
                    //Validate connection established
                    if(rootReply->isAcceptedConnection)
                    {
                        isPaired = TRUE;
                        insertEntry(entryP); //FIXME: create rip entry
                        return SUCCESS;
                    }
                }
                else //Got a packet from other leaf, retry
                {
                    retryN++;
                }
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
  hdr->number = 0;
  while(done <= size)
  {
    hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
    memcpy(packet.data, &msg[done], DATA_SIZE);
    nrf24l01_settxaddr(nfr23l01_pipeAddr(nrf24l01_addr, pipe)); //FIXME: address from RIP table
    nrf24l01_write((uint8_t *)&packet);
    done += 8;
    hdr->number += 1;
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
ret_t getMessageFrom(uint16_t id, char *buf, uint16_t size)
{
  /*
   * We are like a girl in love,
   * just waiting that special someone to call <3
   */
   ret_t ret;
   int retry = 0;
   headerPack_p hdr = (headerPack_p)buf;
   int notHim = 1;
   while(notHim)
   {
      /* we cannot be in love forever </3 */
      if(retry == _MAX_RETRIES_)
        return WARNING;
      ret = getMessage(buf, size);
      if(hdr->idSrc == id)
      {
        notHim = 0;
      }
      retry++;
   }
   return ret;
}

ret_t insertEntry(ripEntry_t *newEntry)
{
    return UNIMPLEMENTED;
}

/* Internal, aux functions */
int isRootId(uint16_t id)
{
    return 0;
}

int isInRange(uint16_t leafPipe, uint16_t rootPipe)
{
    return 0;    
}

uint8_t getRootFromRange(uint16_t pipe)
{
    return 0;
}

/*
 * Function to calculate the checksum of the network packet
 * 
 * param hdr : Header of the packet to calculate the checksum for
 * param msg : Message body of the packet
 * param left: Size of bytes left to send 
 */

uint8_t checksumCalculator(headerPack_p hdr, 
                                   char *msg, 
                                uint8_t left)
{
  uint8_t i, checksum = 0;
  uint8_t limit = left < DATA_SIZE ? left : DATA_SIZE;
  checksum ^= hdr->type;
  checksum ^= hdr->size;
  checksum ^= hdr->ttl;
  checksum ^= hdr->number;
  checksum ^= (uint8_t)((hdr->idSrc & 0xFF00)>>8);
  checksum ^= (uint8_t)(hdr->idSrc & 0x00FF);
  checksum ^= (uint8_t)((hdr->idDest & 0xFF00)>>8);
  checksum ^= (uint8_t)(hdr->idDest & 0x00FF);
  
  for(i = 0; i < limit; i++)
    checksum ^= msg[i];
    
  return checksum;
}
