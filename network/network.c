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
#include <math.h>
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
    uint8_t address;
    uint8_t retryN = 0;   
    char rxData[NRF24L01_PAYLOAD];
    discPack_p tempPacket = (discPack_p) rxData; 
    headerPack_p tempHeader = (headerPack_p) rxData;
    
    //Used by root node, leaf nodes send ripEntries 
    rootReplyP_t myRootReply;
    ripEntry_p entryP = (ripEntry_p)tempPacket->data;    
    
    
    //Used by leaf node, root node sends rootReplies
    ripEntry_t myLeafInfo;
    rootReplyP_p rootReply = (rootReplyP_p)tempPacket->data;      
                                                  
    // --------------------------------------------
    srand(TCNT2);

    gID = rand(); 
    printf("gID %s:%d:%d\n", __FILE__, __LINE__, gID);   
    
    // Select initial address, range 1-255
    address = gID%_MAXIMUM_ADDRESS_+ 1;
    // Validate root
    if(isRootId(gID))
    {  
        while(retryN < _MAX_RETRIES_)
        {
            //Wait for a non-root node in range to send a message
            while( !nrf24l01_readready(_JOIN_PIPE_) );
            //Someone sends us a message  
            nrf24l01_read(rxData);
            if(isInRange(entryP->id, gID)) //leaf in our range
            {
                if(usedEntries < _MAX_PIPES_)
                {
                    insertEntry(entryP); 
                    //Send package to leaf node
                    myRootReply.isAcceptedConnection = TRUE;
                    myRootReply.address = getFreeAddress(address); //Based on my current root address 
                    myRootReply.gID2 = 0;

                    sendMessageTo(tempHeader->idSrc, DISCOVERY,
                              (char *)&myRootReply, sizeof(myRootReply));
                    printf("Sent root reply message %s:%d:%d\n", __FILE__, __LINE__, gID); 
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
        //FIXME: autoconnect roots                              
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
                myLeafInfo.address = address;
                myLeafInfo.isRoot = 0; 
                
                
                sendMessageTo(getRootFromAddr(address), DISCOVERY,
                              (char *)&myLeafInfo, sizeof(myLeafInfo));
                printf("Sent discovery message %s:%d:%d\n", __FILE__, __LINE__, gID);   
            }
            else
            {
                //Verify packet destiny
                nrf24l01_read(rxData);
                if( isRootId(tempHeader->idSrc) )                 
                {                                
                    if( isInRange(gID, tempHeader->idSrc) ) //If the packet is from our root
                    { 
                        //Validate connection established
                        if(rootReply->isAcceptedConnection)
                        {
                            isPaired            = TRUE;  
                            isRoot              = FALSE;
                            /* Reuse leaf entry to fill the unique entry for this leaf */ 
                            myLeafInfo.id       = tempHeader->idSrc;
                            myLeafInfo.address  = rootReply->address;
                            myLeafInfo.isRoot   = FALSE;
                            insertEntry(&myLeafInfo);
                            printf("Success pairing %s:%d:%d:%d:%d\n", __FILE__, __LINE__, gID, tempHeader->idSrc,
                                    rootReply->address);   
                            return SUCCESS;
                        }
                    }
                    else //Packet from other root, simply discard it
                        retryN++;
                }
                else //Got a packet from other leaf
                {
                    if( tempHeader->idSrc == gID)
                    {
                        //global ID collision detected, modify ours
                        gID = rand();  
                        address = gID%255 + 1;
                    }
                    else //Packet from other leaf, simply discard it
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
  uint8_t idx, addr = DEFAULT_PIPE, done = 0, found = 0;
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
      addr = re->address;                                 
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
    addr = re->address;
  }
  hdr->number = 0;
  while(done <= size)
  {
    hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
    memcpy(packet.data, &msg[done], DATA_SIZE);
    nrf24l01_settxaddr(nfr23l01_pipeAddr(nrf24l01_addr, addr));
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
    /* A root ID must be a multiple of _MAX_PIPES_ */
    if(id%(uint16_t)_MAX_PIPES_ == 0)
      return TRUE;
    else
      return FALSE;
}

int isInRange(uint16_t leafID, uint16_t rootID)
{
    if(abs(rootID-leafID) <= (uint16_t)_MAX_PIPES_)
        return TRUE;
    else
        return FALSE;
}

uint8_t getRootFromAddr(uint8_t address)
{
    uint8_t retN = _MAX_PIPES_*floor((float)address/_MAX_PIPES_);
    return retN;
}

uint8_t getFreeAddress(uint8_t rootAddr)
{
   uint8_t i;
   ripEntry_p rep;
   uint8_t startVal = rootAddr;
   for(i=startVal; i < startVal+_MAX_PIPES_; i++)
   {
     //Validate each address in the same range as gID 
     if( !isInRIP(i) )
     {
       return i;
     }
   }
   return 0; //Error, no free address in ranga was found
} 

uint8_t isInRIP(uint8_t inpAddr)
{
    int i;
    ripEntry_p re;
    for(i=0; i<_MAX_PIPES_; ++i)
    {
      re = &ripTable[i];
      if(re->address == inpAddr)
        return TRUE;
    }
    return FALSE;
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
