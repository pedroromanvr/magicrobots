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
    uint8_t retryN;
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
join_retry:
    retryN = 0;
    //gID = rand();     // <--- DELETE_ME
    // Select initial address, range 1-255
    address = GET_ADDRESS(gID);
    printf("gID %d:%d\n", __LINE__, gID);   
    // Validate root
    if(isRootId(gID))
    {  
        printf("Hello I'm root :D\n");
        while(retryN < _MAX_RETRIES_)
        {
            //Wait for a non-root node in range to send a message
            printf("I'm waiting for a leaf...\n");
            while( !nrf24l01_readready(_JOIN_PIPE_) );
            //Someone sends us a message  
            nrf24l01_read(rxData);
            printf("R_Got package, %s:%d:gID:%d:addr:%d:type:%d:size:%d:checksum:%d:ttl:%d:numb:%d:srcID:%d:destID:%d-\n", __FILE__, __LINE__, gID, address, tempHeader->type, tempHeader->size, tempHeader->checksum, tempHeader->ttl, tempHeader->number, tempHeader->idSrc, tempHeader->idDest); 
            if(isInRange(entryP->id, gID)) //leaf in our range
            {
                if(usedEntries < _MAX_PIPES_)
                {
                    printf("Adding leaf to RIP, %s:%d:%d\n", __FILE__, __LINE__, gID); 
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
        //TODO: autoconnect roots                              
    }
    else // Leaf node
    {
        printf("Hello I'm a leaf! D:\n");
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
                
                
                sendMessageTo(getRootFromID(gID), DISCOVERY,
                              (char *)&myLeafInfo, sizeof(myLeafInfo));
                printf("Sent discovery message %s:%d:%d\n", __FILE__, __LINE__, gID);   
            }
            else
            {
                //Verify packet destiny
                nrf24l01_read(rxData);
                printf("L_Got package, %s:%d:gID:%d:addr:%d:type:%d:size:%d:checksum:%d:ttl:%d:numb:%d:srcID:%d:destID:%d-\n", __FILE__, __LINE__, gID, address, tempHeader->type, tempHeader->size, tempHeader->checksum, tempHeader->ttl, tempHeader->number, tempHeader->idSrc, tempHeader->idDest); 
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
                            printf("Success pairing %s:%d:gID:%d:srcID:%d:rootAddr:%d\n", __FILE__, __LINE__, gID, tempHeader->idSrc,
                                    rootReply->address);   
                            return SUCCESS;
                        }
                        //TODO: root did not accept connection
                        break;
                    }
                    else //Packet from other root, simply discard it
                        retryN++;
                }
                else //Got a packet from other leaf
                {
                    if( tempHeader->idSrc == gID)
                    {
                        //global ID collision detected, modify ours
                        goto join_retry;
                    }
                    else //Packet from other leaf, simply discard it
                        retryN++;
                }
            }   
        }
        // MAX number of retries reached, move from range
        //goto join_retry;  // <----- DELETE_ME
    }
    printf("Error, could not pair %s:%d:gID:%d:addr:%d:\n", __FILE__, __LINE__, gID, address);   
    return ERROR;
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
  uint8_t idx, done = 0, found = 0;
  ripEntry_p re; 
  discPack_t packet;
  headerPack_p hdr = (headerPack_p)&packet;
  char address[NRF24L01_ADDRSIZE];

  hdr->type = type;
  hdr->size = size;
  hdr->ttl = DEFAULT_TTL;
  hdr->idSrc = gID;
  hdr->idDest = id;           
  
  printHeader(hdr);
  
  for(idx=0; idx<_MAX_PIPES_; idx++)
  {                          
    re = &ripTable[idx];
    printRipEntry(re);
    if(re->id == INVALID_GID)
      continue;
    if(re->id == id)
    {
      found = 1;
      break;
    }
  }
  printf("sendMessageTo:found=%d\n", found);
  if(!found)
  {
    idx = 0;
nextEntry:
    printf("sendMessageTo:nextEntry: idx=%d", idx);
    if(idx == _MAX_PIPES_)
      return WARNING;
    re = &ripTable[idx];    
    if(re->id == INVALID_GID)
    {
      idx++;
      goto nextEntry;
    }
  }
  printRipEntry(re);

  hdr->number = 0;
  while(done <= size)
  {
    printHeader(hdr);
    hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
    memcpy(packet.data, &msg[done], DATA_SIZE);
    getAddrByPipe(idx, address);
    nrf24l01_settxaddr(address);
    nrf24l01_write((uint8_t *)&packet);
    done += DATA_SIZE;
    hdr->number += 1;
  }
  idx++;
  if(!found)
    goto nextEntry;
  return SUCCESS;
}
//Recieve message from any sender
ret_t getMessage(headerPack_p header, char *buf, uint8_t size)
{
    uint8_t pipe;
    int recieved = 0;
    discPack_t packet;
    headerPack_p hdr = (headerPack_p)&packet;

nextPacket:    
    while(!nrf24l01_readready(&pipe))
    {
        nrf24l01_read((uint8_t *)&packet);
        
        /* Can't handle the size */
        if(hdr->size > size)
            return WARNING;
            
        /* Bad packet */
        if(hdr->checksum != checksumCalculator(hdr, 
                                        packet.data,
                                        hdr->size - recieved))
            return ERROR;
                
        /* First iteration: setup header,
         * on the other ones ignore messages from other idSrc
         */
        if(recieved == 0)
            memcpy(header, hdr, sizeof(headerPack_t));
        else if(header->idSrc != hdr->idSrc)
            goto nextPacket;
        
        memcpy(buf + hdr->number * DATA_SIZE, packet.data, 
               DATA_SIZE > (hdr->size - recieved) ? 
               DATA_SIZE : (hdr->size - recieved));
        recieved += DATA_SIZE;
        if(recieved > hdr->size)
            break;
        else
            goto nextPacket;        
    }
        
    return SUCCESS;
}
//Specify ID to recieve from
ret_t getMessageFrom(headerPack_p header, char *buf, 
                                        uint8_t size)
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
      ret = getMessage(header, buf, size);
      if(ret != SUCCESS)
        return ret;
      if(hdr->idSrc == header->idSrc)
      {
        notHim = 0;
      }
      retry++;
   }
   return ret;
}

ret_t insertEntry(ripEntry_p newEntry)
{
    if(usedEntries < _MAX_PIPES_)
    {
      /* Find free spot available i.e. address == 0 */
      int i;
      for(i=0; i<_MAX_PIPES_; i++)
      {
        if(ripTable[i].address == 0) // Free
        {
          if( !isInRIP(newEntry->address) ) //If address is not currently in table
          {
            memcpy(&ripTable[i], newEntry, sizeof(ripEntry_t)); 
            usedEntries++;
            return SUCCESS;
          }
          else
          {
            //Error, address already in table
            printf("Address already in table %d:%d:%d\n", __LINE__, usedEntries,
                                        newEntry->address);
            return ERROR;

          }
        }
      }
      //No free spot available, should never reach here
      return ERROR;
    }
    else
    {
      printf("ripTable is full %d:%d:%d\n", __LINE__, usedEntries,
                                        newEntry->address);
      return ERROR;
    }
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
    if(getRootFromID(leafID) == rootID)
        return TRUE;
    else
        return FALSE;
}

uint16_t getRootFromID(uint16_t id)
{
    uint16_t retN = _MAX_PIPES_*floor((float)id/_MAX_PIPES_);
    return retN;
}

uint8_t getFreeAddress(uint8_t rootAddr)
{
   uint8_t i;
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

ret_t getAddrByPipe(uint8_t pipe, char *addr)
{
    
    /* pipe 0 has an special address.
       pipe 1 to 5 have the common address we 
       are looking for */
    if(pipe > _MAX_PIPES_)
        return ERROR;
    memcpy(addr, nfr24l01_pipeAddr(nrf24l01_addr, 3), 
                                  NRF24L01_ADDRSIZE);
    addr[NRF24L01_ADDRSIZE - 1] = ripTable[pipe].address;
    return SUCCESS;
}

