#include "ai.h"
#include <string.h>

/* Global variables */
pos_t g_pos = {255, 255, -500};      // Initialize with invalid values
pos_p g_posP = &g_pos;
pos_t historicalPos[MAX_HIST_POS];   // Used to save historical positions
int16_t robotResp[MAX_NO_RESPONSES]; // Save responses of all robots 
uint8_t isValidPos = FALSE; 

ret_t processMyPID(void)
{
    
}

ret_t processResponsePID(void)
{
}

ret_t sendPosMulticast(void)
{
}

uint8_t getResponse(void)
{
}

ret_t getRequest(void)
{
    
}

ret_t readPosition(void)
{
    uint8_t pipe;
    pos_t posFromPC;
    headerPack_t header;
    ret_t ret;
    // If there is something in the pipe 0 of the network
    if (nrf24l01_readready(&pipe)) {
      if (pipe == 0) {
        EXEC_N_CHECK(getMessage(&header, (char *)&posFromPC, 
                                sizeof(pos_t)), ret);
        if (ret == WARNING)
          return WARNING;
        else
        {
          isValidPos = TRUE;
          return SUCCESS;
        }
      }
    }
    else {
      return WARNING;     // Nothing was sent already
    } 
}

ret_t savePosition(void)
{
    static uint8_t ix = 0;
    if (isValidPos) {
      memcpy(&historicalPos[ix], g_posP, sizeof(pos_t));
      ix = (ix + 1) % MAX_HIST_POS;
    } 
}
