
#include "chat.h"

#define BUF_SIZE 100    //  Maximum size of any message sent from PC
#define DATA_FROM_PC  (UCSR0A & (1 << RXC0 ))
#define RX_BUF        UDR0
#define EOL_CHAR      0x0A
#define DATA_FROM_NET (nrf24l01_readready(NULL))
/*
 * Prints message to STDOUT
 * 
 * Param id:  number to be printed (16-bit long)
 * Param msg: Null-terminated string to print
 */
#define PRINT(id,msg) \
      printf("[%x] says: %s\n", (id), (msg))

uint8_t g_user;

ret_t initChat()
{
  g_user = gID;
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
  return SUCCESS;
}
   
ret_t enterRoom()
{
  // Local variables
  char msgBuf[BUF_SIZE];
  ret_t ret;
  uint8_t i = 0 ;

  headerPack_t header;
  // -------------------
  if (joinNetwork() == SUCCESS)
  {
    printf("Success, joined to network, user:%04x\n", g_user);
  }
  else
  {
    printf("Error, could not join network, user:%04x\n", g_user);
    return ERROR;
  }

  // Once in the room, get all messages from our host PC and network
  while(1)
  {
    if (DATA_FROM_PC)
    {
        // Read data and broadcast it
        msgBuf[i] = RX_BUF;
        // If we recieved end of line, send buffer to network
        if(msgBuf[i] == EOL_CHAR)
        {
          msgBuf[i] = '\0'; //Replace EOL with null
          printf("About to send message:%s\n",msgBuf);
          if(strcmp(msgBuf, "Quit") == 0)
          break;
          ret = sendMessage(msgBuf, strlen(msgBuf));
          if(ret != SUCCESS && ret != WARNING)
          {
             printf("sendMessage failed\n");
             return ERROR;
          }
          // Print msg to this PC
          PRINT(g_user, msgBuf);       
          // Clean buffer and index
          memset(msgBuf, 0, BUF_SIZE);
          i = 0;
        }
        else
          i++;
    }
    if (DATA_FROM_NET)
    {
       //Read data and display it to the host PC
       memset(msgBuf, 0, BUF_SIZE);
       ret = getMessage(&header, msgBuf, BUF_SIZE);
       if(ret != SUCCESS && ret != WARNING)
       {
          printf("getMessage failed\n");
       }
       PRINT(header.idSrc, msgBuf);
    }
  }

  return SUCCESS;
}
