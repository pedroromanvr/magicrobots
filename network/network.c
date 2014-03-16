#include "network.h"

ripEntry_t ripTable[_MAX_PIPES_] = {{0,0,0},
                                    {0,0,0},
                                    {0,0,0},
                                    {0,0,0},
                                    {0,0,0}};
uint8_t usedEntries = 0;
uint8_t isPaired = FALSE;
uint8_t isRoot = 255;   //Initialize with any value

uint16_t gID = INVALID_GID;

/*
 * pp must be NULL for leaf nodes
*/
ret_t joinNetworkOnTheFly(discPack_p pp)
{
    ret_t ret;
    uint8_t address;
    uint8_t pipe;
    discPack_p tempPacket;
    headerPack_p tempHeader;

    address = GET_ADDRESS(gID);
    // Validate root or leaf
    if(pp == NULL && isRootId(gID) == TRUE ||
       pp != NULL && isRootId(gID) == FALSE)
    {
        printf("DEBUG=joinNetworkOnTheFly: Invalid packet for gID\n");
        return ERROR;
    }
    if(pp) /* A leaf wants to join the network */
    {
        rootReplyP_t myRootReply;
        ripEntry_p entryP;
        tempPacket = pp;
        tempHeader = (headerPack_p)pp;
        entryP = (ripEntry_p)tempPacket->data;

        netDebugPrint("DEBUG=joinNetworkOnTheFly: recieved a leaf request\n");
        printHeader(tempHeader);
        printRipTable();
        if(!isInRange(entryP->id, gID))
        {
            printf("DEBUG=joinNetworkOnTheFly: Invalid range\n");
            return WARNING;
        }
        if(usedEntries < _MAX_PIPES_)
        {
            //Send package to leaf node
            myRootReply.isAcceptedConnection = TRUE;
            myRootReply.address = getFreeAddress(address); //Based on my current root address
            myRootReply.gID2 = 0;

            printRootReply(&myRootReply);

            netDebugPrint("DEBUG=joinNetworkOnTheFly: Adding leaf to RIP\n");
            //Modify address of entryP to save it in our RIP
            entryP->address = myRootReply.address;
            printRipEntry(entryP);
            ret = insertEntry(entryP);
            if(ret != SUCCESS && ret != WARNING)
            {
                printf("DEBUG=joinNetworkOnTheFly: InsertEntry FAILED");
                return FAIL;
            }
            if(ret == WARNING)
            {
                printf("DEBUG=joinNetworkOnTheFly: Resending rootReply\n");
                entryP = ripTable;
                while(entryP->id != pp->header.idSrc)
                    entryP++;
                myRootReply.address = entryP->address;
                //return WARNING;
            }

            tempHeader->type = DISCOVERY;
            tempHeader->size = sizeof(myRootReply);
            tempHeader->ttl = DEFAULT_TTL;
            tempHeader->number = 0;
            tempHeader->idDest = tempHeader->idSrc;
            tempHeader->idSrc = gID;

            netDebugPrint("DEBUG=joinNetworkOnTheFly: Sent root reply message:%d\n", gID);
            ret = sendMessageTo(tempHeader, (char *)&myRootReply,
                                sizeof(myRootReply));

            if(ret != SUCCESS && ret != WARNING)
            {
                printf("DEBUG=joinNetworkOnTheFly: sendMessageTo failed\n");
                return WARNING;
            }
            else
            {
                isPaired = TRUE;
                printf("DEBUG=joinNetworkOnTheFly: Root SUCCESS!\n");
                return SUCCESS;
            }
        }
        else
        {
            /* Maybe a leaf that already was connected wants to re-join.
               Just resend the myRootReply
             */
            printf("DEBUG=joinNetworkOnTheFly: sorry, not yet implemented\n");
            return UNIMPLEMENTED;
            //Convert leaf node to root
            //break outter while
        }
        //TODO: autoconnect roots
    }
    else /* I want to join the network */
    {
        ripEntry_t myLeafInfo;
        rootReplyP_p rootReply;
        uint32_t cnt = 0;
        uint8_t txData[NRF24L01_PAYLOAD];
        tempPacket = (discPack_p)txData;
        tempHeader = (headerPack_p)txData;
        rootReply = (rootReplyP_p)tempPacket->data;

        myLeafInfo.id   = gID;
        myLeafInfo.address = address;
        myLeafInfo.isRoot = 0;

        printRipEntry(&myLeafInfo);

        //Check if someone has just sent something
        if( !nrf24l01_readready(&pipe) )
        {
            netDebugPrint("DEBUG=joinNetworkOnTheFly: Sent discovery message to root\n");
            tempHeader->type = DISCOVERY;
            tempHeader->size = sizeof(myLeafInfo);
            tempHeader->ttl = DEFAULT_TTL;
            tempHeader->number = 0;
            tempHeader->idSrc = gID;
            tempHeader->idDest = getRootFromID(gID);
            ret = sendMessageTo(tempHeader, (char *)&myLeafInfo,
                                sizeof(myLeafInfo));
            if(ret != SUCCESS && ret != WARNING)
            {
                printf("DEBUG=joinNetworkOnTheFly: sendMessageTo FAILED\n");
                return WARNING;
            }

            netDebugPrint("DEBUG=joinNetworkOnTheFly: Waiting for root answer.\n");
            while(!nrf24l01_readready(&pipe) && cnt != DEFAULT_TIMEOUT)
                cnt += 1;
            if(cnt == DEFAULT_TIMEOUT)
            {
                netDebugPrint("DEBUG=joinNetworkOnTheFly: Timeout.\n");
                return WARNING;
            }
            nrf24l01_read(txData);
            netDebugPrint("DEBUG=joinNetworkOnTheFly: Message recieved.\n");

            if(tempHeader->type != DISCOVERY ||
               tempHeader->idDest != gID ||
               tempHeader->idSrc != getRootFromID(gID))
            {
                netDebugPrint("DEBUG=joinNetworkOnTheFly: The message seems to be not for us, retrying...\n");
                return SUCCESS;
            }

            /* We can assume we recieved a rootReply */
            if(rootReply->isAcceptedConnection)
            {
                isPaired            = TRUE;
                isRoot              = FALSE;
                /* Reuse leaf entry to fill the unique entry for this leaf */
                myLeafInfo.id       = tempHeader->idSrc;
                myLeafInfo.address  = rootReply->address;
                myLeafInfo.isRoot   = TRUE;
                insertEntry(&myLeafInfo);
                printRipTable();
                printf("DEBUG=joinNetworkOnTheFly: Leaf SUCCESS\n");
                return SUCCESS;
            }
            else
            {
                printf("DEBUG=joinNetworkOnTheFly: Root rejected the connection\n");
                return WARNING;
                //TODO: root did not accept connection D:
            }
        }

        // MAX number of retries reached, move from range
        return UNIMPLEMENTED;
    }
    printf("DEBUG=joinNetworkOnTheFly: Error, could not pair: gID:%d:addr:%d:\n",
           gID, address);
    return WARNING;
}

//Broadcast message originated in this host
ret_t sendMessage(char *msg,  uint8_t size)
{
  uint8_t idx;
  ripEntry_p re;
  headerPack_t hdr;

  hdr.type = BROADCAST;
  hdr.idSrc = gID;
  hdr.ttl = DEFAULT_TTL;

  for(idx=0; idx<_MAX_PIPES_; idx++)
  {
    re = &ripTable[idx];
    if(re->id == INVALID_GID)
      continue;
    hdr.idDest = re->id;
    netDebugPrint("DEBUG=sendMessage: Calling sendMessageTo for idx=%d.\n", idx);
    sendMessageTo(&hdr, msg, size);
  }
    return SUCCESS;
}
ret_t forwardMessage(headerPack_p header,
                     char *msg,  uint8_t size)
{
  uint8_t idx;
  ripEntry_p re;

  for(idx=0; idx<_MAX_PIPES_; idx++)
  {
    re = &ripTable[idx];
    if(re->id == INVALID_GID)
      continue;
    header->idDest = re->id;
    netDebugPrint("DEBUG=forwardMessage: Calling sendMessageTo for idx=%d.\n", idx);
    sendMessageTo(header, msg, size);
  }
    return SUCCESS;
}
/*
 * Advanced way to send a message.
 * No verification of any type is done.
 * header + msg are transmited in one packet.
 */
ret_t microSendMessage(headerPack_p header,
                     char *msg,  uint8_t size)
{
    discPack_t packet;
    memcpy(&(packet.header), header, sizeof(headerPack_t));
    memcpy(packet.data, msg, size);                        
    nrf24l01_write((uint8_t *)&packet);    
    netDebugPrint("DEBUG=microSendMessage: Message Sent.\n");
    return SUCCESS;
}
//Send message to an specific ID
ret_t sendMessageTo(headerPack_p header,
                    char *msg, uint8_t size)
{
  uint8_t idx=0, done = 0, found = 0;
  ripEntry_p re;
  discPack_t packet;
  headerPack_p hdr = (headerPack_p)&packet;
  char address[NRF24L01_ADDRSIZE];

  memcpy(hdr, header, sizeof(headerPack_t));

  hdr->ttl -= 1;
  netDebugPrint("DEBUG=sendMessageTo: start.\n");

  printHeader(hdr);

  if(hdr->type == DISCOVERY)
  {
    /* DISCOVERY packets are special, since are sent at the beggining.
     * ripTable is presumably empty so we basically just send the message to a
     * default pipe. Hopefully a root will be waiting for us.
     * A DISCOVERY message should be shorter than DATA_SIZE
      */
     netDebugPrint("DEBUG=sendMessageTo: DISCOVERY packet\n");
     if(size > DATA_SIZE)
     {
        printf("DEBUG=sendMessageTo: DATA_SIZE exceeded\n");
        return ERROR;
     }
     hdr->checksum = checksumCalculator(hdr,
                       &msg[0], size);
     memcpy(packet.data, msg, DATA_SIZE);
     memcpy(address, nrf24l01_addr0, NRF24L01_ADDRSIZE);
     printHeader(hdr);
     nrf24l01_settxaddr(address);
     nrf24l01_write((uint8_t *)&packet);
    netDebugPrint("DEBUG=sendMessageTo:DISCOVERY packet sent!\n");
    return SUCCESS;
  }
  if(hdr->type == BROADCAST)
  {
    netDebugPrint("DEBUG=sendMessageTo: BROADCAST message detected!\n");
    goto nextEntry;
  }
  for(idx=0; idx<_MAX_PIPES_; idx++)
  {
    re = &ripTable[idx];
    printRipEntry(re);
    if(re->id == INVALID_GID)
      continue;
    if(re->id == hdr->idDest)
    {
      found = 1;
      break;
    }
  }
  //netDebugPrint("sendMessageTo:found=%d\n", found);
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
  //netDebugPrint("sendMessageTo:nextEntry: idx=%d\n", idx);
  printRipEntry(re);

  /* First send an empty header to inform the reciever
   * about our willing to transmit
   */
  hdr->number = 0;
  hdr->size = 0;
  hdr->checksum = checksumCalculator(hdr,
                     msg, 0);
  getAddrByPipe(idx, address);
  nrf24l01_settxaddr(address);
  nrf24l01_write((uint8_t *)&packet);
  _delay_ms(DEFAULT_DELAY);
  netDebugPrint("DEBUG=sendMessageTo: Empty header sent!\n");
  printHeader(hdr);

  hdr->size = size;
  netDebugPrint("DEBUG=sendMessageTo: message loop start!\n");
  while(done <= size)
  {
    hdr->checksum = checksumCalculator(hdr,
                       &msg[done], size - done);
    printHeader(hdr);
    memcpy(packet.data, &msg[done], DATA_SIZE);
    getAddrByPipe(idx, address);
    nrf24l01_settxaddr(address);
    nrf24l01_write((uint8_t *)&packet);
    netDebugPrint("sendMessageTo: Packet sent!\n");
    _delay_ms(DEFAULT_DELAY);
    done += DATA_SIZE;
    hdr->number += 1;
  }
  idx++;
  if(!found)
    goto nextEntry;
  netDebugPrint("DEBUG=sendMessageTo: SUCCESS!\n");
  return SUCCESS;
}
//Recieve message from any sender
ret_t getMessage(headerPack_p header, char *buf, uint8_t size)
{
    uint8_t pipe = 255;
    uint32_t cnt = 0;
    uint8_t recieved = 0;
    discPack_t packet;
    headerPack_p hdr = (headerPack_p)&packet;

    while(1)
	{
		netDebugPrint("DEBUG=getMessage: waiting for packet...\n");
		while(!nrf24l01_readready(&pipe) && cnt != DEFAULT_TIMEOUT)
			cnt += 1;
		if(cnt == DEFAULT_TIMEOUT)
		{
			netDebugPrint("DEBUG=getMessage: Timeout.\n");
			return ERROR;
		}
		if(pipe != 0)
		{
			nrf24l01_read((uint8_t *)&packet);
			netDebugPrint("DEBUG=getMessage: packet recieved!\n");

			printHeader(hdr);

			if(hdr->size > size)
			{
				printf("DEBUG=getMessage: packet size too big.\n");
				return WARNING;
			}

			if(hdr->checksum != checksumCalculator(hdr,
											packet.data,
											hdr->size - recieved))
			{
				printf("DEBUG=getMessage: packet checksum error!\n");
				return WARNING;
			}
			if(recieved == 0)
			{
				netDebugPrint("DEBUG=getMessage: First packet, storing header.\n");
				memcpy(header, hdr, sizeof(headerPack_t));
			}
			else if(header->idSrc != hdr->idSrc)
			{
				netDebugPrint("DEBUG=getMessage: The packet doesn't belong to this message.\n");
				continue;
			}
			memcpy(buf + (hdr->number * DATA_SIZE), packet.data,
				   (hdr->size - recieved) > DATA_SIZE ?
				   DATA_SIZE : (hdr->size - recieved));
			recieved += DATA_SIZE;
			netDebugPrint("DEBUG=getMessage: copied data.\n");

			if(recieved < hdr->size)
				netDebugPrint("DEBUG=getMessage: Still data left.\n");
			else
				break;
		}
		else
		{
			netDebugPrint("DEBUG=getMessage: Incorrect pipe.\n");
			continue;
		}
	}
    netDebugPrint("DEBUG=getMessage: SUCCESS\n");
    return SUCCESS;
}

//Specify ID to recieve from
ret_t getMessageFrom(headerPack_p header, char *buf,
                                        uint8_t size)
{
   ret_t ret;
   uint8_t retry = 0;
   uint16_t idSrc = header->idSrc;
   int notHim = 1;
   while(notHim)
   {
      if(retry == _MAX_RETRIES_)
      {
        return ERROR;
	netDebugPrint("DEBUG=getMessageFrom: Max retry's reached.\n");
      }
      ret = getMessage(header, buf, size);
      if(ret != SUCCESS || ret != WARNING)
        return ret;
      if(idSrc == header->idSrc)
      {
	netDebugPrint("DEBUG=getMessageFrom: Incorrect idSrc, retrying.\n");
        notHim = 0;
      }
      retry++;
   }
   return SUCCESS;
}

/* SUCCESS: rip entry inserted.
   WARNING: rip entry NOT inserted.
   Other: Something bad happened
 */
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
            /* This is not catastrophic */
            printf("DEBUG=insertEntry: ripEntry id already present in ripTable\n");
            printRipEntry(newEntry);
            printRipEntry(&(ripTable[i]));
            return WARNING;
        }

      }
      if(isInRIP(newEntry->address))
      {
        //Error, address already in table
        /* This is not catastrophic */
        printf("DEBUG=insertEntry: Address already in table %d:%d:%d\n",
        __LINE__, usedEntries, newEntry->address);
        printRipEntry(&(ripTable[i]));
        return WARNING;
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
        printf("DEBUG=insertEntry: Inconsistant usedEntries value\n");
      return ERROR;
    }
    else
    {
      printf("DEBUG=ripTable is full %d:%d:%d\n",
             __LINE__, usedEntries, newEntry->address);
      printRipTable();
      return ERROR;
    }
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

/* if we return WARNING it means that something was placed
 * in the buffer. You must read it.
 * if we return SUCCESS the  no user action is required.
 * if we return ERROR something went wrong, probably during
 * transmission. It is not outstanding.
*/
ret_t networkManager(headerPack_p header,
              char *buffer, uint8_t size)
{
    discPack_t packet;
    char pipe;
    ret_t ret;
    memset(&packet, 0, sizeof(discPack_t));

    if(gID == INVALID_GID)
    {
        gID = (uint16_t)random8();
        netDebugPrint("DEBUG=gID %d:%d\n", __LINE__, gID);
        if(!isRootId(gID))
            netDebugPrint("DEBUG=Hello I'm a leaf D:\n");
        else
            netDebugPrint("DEBUG=Hello I'm root :D\n");
    }
    if(!isPaired && !isRootId(gID))
    {
        netDebugPrint("DEBUG=networkManager: calling joinNetworkOnTheFly for leaf\n");
        joinNetworkOnTheFly(NULL);
    }
    if(nrf24l01_readready(&pipe))
    {
        netDebugPrint("DEBUG=networkManager: packet recieved!\n");
        nrf24l01_read((uint8_t *)&packet);
        printHeader(&(packet.header));
        if(packet.header.checksum != checksumCalculator(&(packet.header), packet.data, packet.header.size))
        {
            netDebugPrint("DEBUG=networkManager: checksum faield!\n");
            return SUCCESS;
        }
        if(packet.header.idDest != gID)
        {
            netDebugPrint("DEBUG=networkManager: The message is not for us!\n");
            return SUCCESS;
        }
        if(packet.header.type == DISCOVERY)
        {
            netDebugPrint("DEBUG=networkManager: DISCOVERY packet.\n");
            ret = joinNetworkOnTheFly(&packet);
            if(ret != WARNING && ret != SUCCESS)
            {
                printf("DEBUG=networkManager: joinNetworkOnTheFly FAILED\n");
                return ERROR;
            }
            else
                return SUCCESS;
        }
        if(packet.header.type == DATA)
        {
            netDebugPrint("DEBUG=networkManager: DATA packet.\n");
            if(packet.header.size <= size)
            {
                ret = getMessageFrom(header, buffer, size);
                if(ret != WARNING && ret != SUCCESS)
                {
                    printf("DEBUG=networkManager: getMessageFrom FAILED\n");
                    return ERROR;
                }
                return WARNING;
            }
            else
            {
                printf("DEBUG=networkManager: DATA size exceeded\n");
                return ERROR;
            }
        }
        if(packet.header.type == RAW)
        {
            netDebugPrint("DEBUG=networkManager: RAW packet.\n");
            if(size >= DATA_SIZE)
            {
                memcpy(buffer, packet.data, DATA_SIZE);
                memcpy(header, &packet, sizeof(headerPack_t));
                return WARNING;
            }
            else
            {
                printf("DEBUG=networkManager: RAW size exceeded\n");
                return ERROR;
            }
        }
        if(packet.header.type == BROADCAST)
        {
            if(packet.header.idSrc == gID)
                return SUCCESS;
            netDebugPrint("DEBUG=networkManager: BROADCAST packet.\n");
            if(packet.header.size <= size)
            {
                ret = getMessageFrom(header, buffer, size);
                if(ret != SUCCESS && ret != WARNING)
                {
                    printf("DEBUG=networkManager: getMessageFrom FAILED\n");
                    return ret;
                }
                if(ret == SUCCESS)
                {
                    forwardMessage(header, buffer, packet.header.size);
                    return WARNING;
                }
            }
            else
            {
                printf("DEBUG=networkManager: BROADCAST size exceeded\n");
                return ERROR;
            }
        }
        netDebugPrint("DEBUG=networkManager: UNKNOWN packet.\n");

    }
    return SUCCESS;
}
