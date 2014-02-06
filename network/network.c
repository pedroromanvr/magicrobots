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

#if MLEONAY == 1
uint16_t gID = 0xf1;   
#else
uint16_t gID = 0xf0;   
#endif

#include "../movement/movement.h"
#include "platform.h"
#include "stdlib.h"
#include <math.h>
#include <delay.h>

#define _JOIN_PIPE_ 0
#define _MAX_RETRIES_ 100
#define _DELTA_DELAY_ 10


//To use outside this API
ret_t joinNetwork()
{
    // ----------- Variable declarations ----------
    ret_t ret;
    uint8_t address;
    uint8_t retryN;
    uint8_t pipe;
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
            //printRipTable();
            printf("I'm waiting for a leaf...\n");
            while( !nrf24l01_readready(_JOIN_PIPE_) );
            //Someone sends us a message  
            nrf24l01_read(rxData);
            printf("R_Got package, %s:%d:gID:%d:addr:%d:type:%d:size:%d:checksum:%d:ttl:%d:numb:%d:srcID:%d:destID:%d-\n", __FILE__, __LINE__, gID, address, tempHeader->type, tempHeader->size, tempHeader->checksum, tempHeader->ttl, tempHeader->number, tempHeader->idSrc, tempHeader->idDest); 
            if(isInRange(entryP->id, gID)) //leaf in our range
            {
                if(usedEntries < _MAX_PIPES_)
                {                              
                    //Send package to leaf node
                    myRootReply.isAcceptedConnection = TRUE;
                    myRootReply.address = getFreeAddress(address); //Based on my current root address 
                    myRootReply.gID2 = 0;                              
                    
                    printRootReply(&myRootReply);    
                    
                    printf("Adding leaf to RIP, %s:%d:%d\n", __FILE__, __LINE__, gID);   
                    //Modify address of entryP to save it in our RIP
                    entryP->address = myRootReply.address;
                    printRipEntry(entryP); 
                    ret = insertEntry(entryP);
                    if(ret != SUCCESS)
                        printf("InsertEntry FAILED");

                    ret = sendMessageTo(tempHeader->idSrc, DISCOVERY,
                              (char *)&myRootReply, sizeof(myRootReply));
                    if(ret != SUCCESS && ret != WARNING)
                    {
                        printf("sendMessageTo failed\n");
                        return ERROR;
                    }
                    else
                    {
                        printf("Sent root reply message %s:%d:%d\n", __FILE__, __LINE__, gID); 
                        return SUCCESS;
                    }
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
            if( !nrf24l01_readready(&pipe) )
            {
                //Send a message to our root, build packet to send
                myLeafInfo.id   = gID;
                myLeafInfo.address = address;
                myLeafInfo.isRoot = 0;
                
                printf("Send a message to our root\n");
                printRipEntry(&myLeafInfo);
                
                ret = sendMessageTo(getRootFromID(gID), DISCOVERY,
                              (char *)&myLeafInfo, sizeof(myLeafInfo));
                if(ret != SUCCESS && ret != WARNING)
                {
                    printf("sendMessageTo FAILED\n");
                    retryN++;
                }
                else
                    printf("Sent discovery message %s:%d:%d\n", __FILE__, __LINE__, gID);
                
                _delay_ms(100);
            }
            else
            {
                //Verify packet destiny
                nrf24l01_read(rxData);
                printf("L_Got package, %d:pipe:%d:gID:%d:addr:%d:type:%d:size:%d:checksum:%d:ttl:%d:numb:%d:srcID:%d:destID:%d-\n", 
                        __LINE__, pipe, gID, address, tempHeader->type, tempHeader->size, tempHeader->checksum, tempHeader->ttl, tempHeader->number, tempHeader->idSrc, tempHeader->idDest); 
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
                            myLeafInfo.isRoot   = TRUE;
                            insertEntry(&myLeafInfo);
                            //printRipTable();   
                            printf("Success pairing %d:gID:%d:srcID:%d:rootAddr:%d\n", 
                                    __LINE__, gID, tempHeader->idSrc, rootReply->address);
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
  uint8_t idx=0, done = 0, found = 0;
  ripEntry_p re; 
  discPack_t packet;
  headerPack_p hdr = (headerPack_p)&packet;
  char address[NRF24L01_ADDRSIZE];   
  
  printf("sendMessageTo start\n");

  hdr->type = type;
  hdr->size = size;
  hdr->ttl = DEFAULT_TTL;
  hdr->idSrc = gID;
  hdr->idDest = id;           
  hdr->number = 0;
  
  printHeader(hdr);             
  
  if(type == DISCOVERY)
  {
    /* DISCOVERY packets are special, since are sent at the beggining.
     * ripTable is presumably empty so we basically just send the message to a
     * default pipe. Hopefully a root will be waiting for us.
     * A DISCOVERY message should be shorter than DATA_SIZE
      */
     printf("sendMessageTo:DISCOVERY packet\n");
     if(size > DATA_SIZE)
     {
        printf("sendMessageTo:DATA_SIZE exceeded\n");
        return ERROR;
     }
     hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
     memcpy(packet.data, msg, DATA_SIZE);
     memcpy(address, nrf24l01_addr0, NRF24L01_ADDRSIZE);
     nrf24l01_settxaddr(address);
     nrf24l01_write((uint8_t *)&packet);
    printf("sendMessageTo:DISCOVERY packet sent!\n");
    return SUCCESS;
  }
  if(type == BROADCAST)
  {
    printf("sendMessageTo: BROADCAST message detected!\n");
    goto nextEntry;
  }
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
    if(idx == _MAX_PIPES_)
      return WARNING;
    re = &ripTable[idx];    
    if(re->id == INVALID_GID)
    {
      idx++;
      goto nextEntry;
    }
  }
  printf("sendMessageTo:nextEntry: idx=%d\n", idx);

  printRipEntry(re);

  hdr->number = 0;
  while(done <= size)
  {
    hdr->checksum = checksumCalculator(hdr, 
                       &msg[done], size - done);  
    printHeader(hdr);
    memcpy(packet.data, &msg[done], DATA_SIZE);
    getAddrByPipe(idx, address);
    nrf24l01_settxaddr(address);
    nrf24l01_write((uint8_t *)&packet);
    _delay_ms(1);
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
    printf("getMessage: waiting for packet...\n");
    while(!nrf24l01_readready(&pipe))
        _delay_ms(10);
    if(pipe != 0)
    {
        nrf24l01_read((uint8_t *)&packet);    
        printf("getMessage: packet recieved!\n");

        printHeader(hdr);
        
        /* Can't handle the size */
        if(hdr->size > size)
            return WARNING;
        printf("getMessage: packet size OK!\n");
            
        /* Bad packet */
        if(hdr->checksum != checksumCalculator(hdr, 
                                        packet.data,
                                        hdr->size - recieved))
            return ERROR;
        printf("getMessage: packet checksum correct!\n");
                
        /* First iteration: setup header,
         * on the other ones ignore messages from other idSrc
         */
        if(recieved == 0)
            memcpy(header, hdr, sizeof(headerPack_t));
        else if(header->idSrc != hdr->idSrc)
            goto nextPacket;
        
        memcpy(buf + hdr->number * DATA_SIZE, packet.data, 
               (hdr->size - recieved) > DATA_SIZE ? 
               DATA_SIZE : (hdr->size - recieved));
        recieved += DATA_SIZE;
        if(recieved < hdr->size)
            goto nextPacket;        
    }
    else
        goto nextPacket;    
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
      char tmpBuf[NRF24L01_ADDRSIZE];
      for(i=0; i<_MAX_PIPES_; i++)
      {
        if(newEntry->id == ripTable[i].id)
        {
            printf("ripEntry id already present in ripTable\n");
            printRipEntry(newEntry);
            printRipEntry(&(ripTable[i]));
            return ERROR;
        }

      }
      if(isInRIP(newEntry->address))
      {
        //Error, address already in table
        printf("Address already in table %d:%d:%d\n", 
        __LINE__, usedEntries, newEntry->address);
        printRipEntry(&(ripTable[i]));
        return ERROR;
      }
      for(i=0; i<_MAX_PIPES_; i++)
      {
        if(ripTable[i].address == 0) // Free
        {
            memcpy(&ripTable[i], newEntry, sizeof(ripEntry_t)); 
             //Configure address in RF module
            getAddrByPipe(i, tmpBuf);
            nrf24l01_setrxaddr(i+1, tmpBuf);
            usedEntries++;
            return SUCCESS;
        }
      }
      //No free spot available, should never reach here
      while(1)
        printf("insertEntry: Inconsistant usedEntries value\n");
      return ERROR;
    }
    else
    {
      printf("ripTable is full %d:%d:%d\n", 
             __LINE__, usedEntries, newEntry->address);     
      printRipTable();
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

uint8_t checksumCalculator(headerPack_p hdr, char *msg, uint8_t left)
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
  printf("checksumCalculator\nlimit:%d\nsize:%d\nchecksum:%d\n\n", 
         limit, hdr->size, checksum);    
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

