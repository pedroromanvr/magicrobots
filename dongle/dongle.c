#include "dongle.h"


ret_t dongleInit()
{
  // USART initialization
  // Communication Parameters: 8 Data, 1 Stop, No Parity
  // USART Receiver: On
  // USART Transmitter: On
  // USART0 Mode: Asynchronous
  // USART Baud Rate: 38400
  UCSR0A=0x00;
  UCSR0B=0x18;
  UCSR0C=0x06;
  UBRR0H=0x00;
  UBRR0L=0x0C;
  
  dongleDebugPrint("DEBUG=dongleInit: Serial port ready.\n");
  /* Turn ON nfr module */
  INIT_NW_STACK();    
  gID = (uint16_t)random8();
  dongleDebugPrint("DEBUG=dongleInit: Ready to stablish peer to peer conections. gID = %d\n",gID);
  return SUCCESS;
}

ret_t dongleMainThread()
{
  // Local variables
  char msgBuf[sizeof(discPack_t) + 1];
  char buffer[sizeof(discPack_t) + 1];
  ret_t ret;
  uint8_t i;        
  uint8_t j;        

  discPack_p   pkt2 = (discPack_p)buffer;
  discPack_p   pkt = (discPack_p)msgBuf;
  
  i =  DATA_FROM_PC;
  
  memset(buffer, 0, sizeof(buffer));
  dongleDebugPrint("DEBUG=dongleMainThread: Start.\n");
  while(1)
  {
    ret = networkManager(&(pkt2->header), pkt2->data, DATA_SIZE);
    if(ret != SUCCESS && ret != WARNING)
    {
        dongleDebugPrint("DEBUG=dongleMainThread: networkManager failed.\n");
    }
    if (ret == WARNING)
    {
       //networkManager read a locatioRequest_t
       dongleDebugPrint("DEBUG=dongleMainThread: Data recieved.\n");
       //dumpPacket((discPack_p)buffer);
       for(j = 0; j < sizeof(discPack_t); j++)
        putchar(buffer[j]);
       memset(buffer, 0, sizeof(buffer));
    }
    if (isPaired && DATA_FROM_PC)
    {
        msgBuf[0] = RX_BUF;
        dongleDebugPrint("DEBUG=dongleMainThread: reading RESPONSE.\n");         

        for(i = 1; i <sizeof(discPack_t); i++)
        {
            //dongleDebugPrint("DEBUG=i=%d.\n", i);         
            msgBuf[i] = getchar();
        }
        //dumpPacket((discPack_p)msgBuf);
        
        pkt->header.checksum = checksumCalculator(&(pkt->header), 
                                   pkt->data, 
                                   sizeof(locationRequest_t));
        dongleDebugPrint("DEBUG=dongleMainThread: calling microSendMessage.\n");
        microSendMessage(&(pkt->header), pkt->data, sizeof(locationRequest_t));          

        // Clean buffer and index
        memset(msgBuf, 0, sizeof(msgBuf));
    }
  }
  return SUCCESS;
}
