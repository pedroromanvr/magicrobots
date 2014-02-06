#ifndef __CHAT__
#define __CHAT__

#include <stdint.h>
#include <string.h>
#include "../platform.h"
#include "../network/network.h"

/* For this chat example, the gID; a 16-bit number defined in network.h,
   will be used as the user of the chat.
   
   This example is intended to test connectivity and failure management, so 
   the chat will have as users all members in the room, in this case, joined 
   to the Network.

   Messages sent by any node will be broadcasted to all members in the
   network. 
   
   Environment setup:
   PC => RS232 => uC => RF Module X => RF Module Y => uC => RS232 => PC 
                                    => RF Module Z => uC => RS232 => PC 
                                    => RF Module . => uC => RS232 => PC 

   The RS232 interface for the PC was tested using CoolTerm, due to its 
   feature "Send String <ctrl+t>", which was suitable to be used simulating
   a chat text-box input. CoolTerm can be downloaded from: 
                                      http://freeware.the-meiers.org/
*/
   
   extern uint8_t g_user;

/* == Functions to be used externally == */


   /* Inits the chat context, including USART module
      returns ret_t (SUCCCESS,ERROR,WARNING)
   */
   extern ret_t initChat();
   
   /* This function contains all the process of the chat itself.
      
      returns ret_t (SUCCCESS,ERROR,WARNING)
   */
   extern ret_t enterRoom();

#endif 
