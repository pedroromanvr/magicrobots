
#include "chat.h"

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
  char buffer[BUF_SIZE];
  ret_t ret = SUCCESS;
  uint8_t i = 0 ;

  headerPack_t header;
  memset(&header, 0, sizeof(headerPack_t));
  // Once in the room, get all messages from our host PC and network
  printf("Welcome to chatRoom!\n");
  while(1)
  {
    g_user = gID;
    ret = networkManager(&header, buffer, BUF_SIZE);
    if(ret != SUCCESS && ret != WARNING)
    {
        printf("enterRoom: networkManager failed!\n");
    }
    if (ret == WARNING)
    {
       //networkManager read something,
       //Display it to the host PC
       PRINT(header.idSrc, buffer);
       memset(buffer, 0, BUF_SIZE);
    }
    if (isPaired && DATA_FROM_PC)
    {
        // Read data and broadcast it
        msgBuf[i] = RX_BUF;
        // If we recieved end of line, send buffer to network
        if (msgBuf[i] == EOL_CHAR || i == (BUF_SIZE - 1))
        {
          msgBuf[i] = '\0'; //Replace EOL with null
          //printf("enterRoom: About to send message:%s\n",msgBuf);
          if(strcmp(msgBuf, "Quit") == 0)
          break;
          ret = sendMessage(msgBuf, strlen(msgBuf));
          if(ret != SUCCESS && ret != WARNING)
             printf("sendMessage failed\n");
          // Print msg to this PC
          PRINT(g_user, msgBuf);
          // Clean buffer and index
          memset(msgBuf, 0, BUF_SIZE);
          i = 0;
        }
        else
          i++;
    }
  }
  return SUCCESS;
}

ret_t enterMicroRoom()
{
  // Local variables
  char msgBuf[BUF_SIZE];
  char buffer[BUF_SIZE];
  ret_t ret = SUCCESS;
  uint8_t i = 0 ;        

  headerPack_t header;
  memset(&header, 0, sizeof(headerPack_t));
  // Once in the room, get all messages from our host PC and network
  printf("Welcome to chatRoom!\n");
  while(1)
  {
    g_user = gID;
    ret = networkManager(&header, buffer, BUF_SIZE);
    if(ret != SUCCESS && ret != WARNING)
    {
        printf("enterRoom: networkManager failed!\n");
    }
    if (ret == WARNING)
    {
       //networkManager read something,
       //Display it to the host PC
       PRINT(header.idSrc, buffer);
       memset(buffer, 0, BUF_SIZE);
    }
    if (isPaired && DATA_FROM_PC)
    {
        // Read data and broadcast it
        msgBuf[i] = RX_BUF;
        // If we recieved end of line, send buffer to network
        if (msgBuf[i] == EOL_CHAR || i == (BUF_SIZE - 1))
        {
          msgBuf[i] = '\0'; //Replace EOL with null
          //printf("enterRoom: About to send message:%s\n",msgBuf);
          if(strcmp(msgBuf, "Quit") == 0)
          break;


          header.type = RAW;
          header.size = i;
          header.ttl = 5;
          header.number = 0;
          header.idSrc = gID;
          if(gID == 240)
            header.idDest = 241;
          else
            header.idDest = 240;
          header.checksum = checksumCalculator(&header, msgBuf, i);
          microSendMessage(&header, msgBuf, i);
          

          // Print msg to this PC
          PRINT(g_user, msgBuf);
          // Clean buffer and index
          memset(msgBuf, 0, BUF_SIZE);
          i = 0;
        }
        else
          i++;
    }
  }
  return SUCCESS;
}
