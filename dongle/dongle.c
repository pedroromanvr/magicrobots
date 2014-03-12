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

  headerPack_p hdr = (headerPack_p)msgBuf;
  discPack_p   pkt = (discPack_p)msgBuf;

  dongleDebugPrint("DEBUG=dongleMainThread: Start.\n");
  while(1)
  {
    ret = networkManager(hdr, buffer, sizeof(buffer));
    if(ret != SUCCESS && ret != WARNING)
    {
        dongleDebugPrint("DEBUG=dongleMainThread: networkManager failed.\n");
    }
    if (ret == WARNING)
    {
       //networkManager read a locatioRequest_t
       dongleDebugPrint("DEBUG=dongleMainThread: Data recieved.\n");
       for(j = 0; j < sizeof(discPack_t); j++)
        putchar(buffer[j]);
       memset(buffer, 0, sizeof(buffer));
    }
    if (isPaired && DATA_FROM_PC)
    {
        dongleDebugPrint("DEBUG=dongleMainThread: reading RESPONSE.\n");

        for(i = 0; i <sizeof(discPack_t); i++)
            msgBuf[i] = getchar();
        
        hdr->checksum = checksumCalculator(hdr, msgBuf, sizeof(discPack_t));
        dongleDebugPrint("DEBUG=dongleMainThread: calling microSendMessage.\n");
        microSendMessage(hdr, pkt->data, sizeof(locationRequest_t));          

        // Clean buffer and index
        memset(msgBuf, 0, sizeof(msgBuf));
    }
  }
  return SUCCESS;
}
