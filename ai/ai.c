#include "ai.h"
#include <string.h>

/* Global variables */
pos_t g_pos = {255, 255, -500};      // Initialize with invalid values
pos_p g_posP = &g_pos;
pos_t historicalPos[MAX_HIST_POS];   // Used to save historical positionspos_t
uint8_t histSize = 0;
int16_t robotResp[MAX_NO_RESPONSES]; // Save responses of all robots
uint8_t respSize = 0;
uint8_t weightRes[MAX_NO_RESPONSES]; // Save weights calculated
uint8_t isValidPos = FALSE;

pidCtx_t pidCtx;                     // PID Context
uint8_t isCTXInit = FALSE;

int16_t desiredAngle = 0;

ret_t processMyPID(void)
{
    uint8_t i;
    int16_t delta, pid_ret;
    if (isCTXInit = FALSE)
    {
       setPIDVals();
    }
    sortResp();
    // robotResp must be ordered 
    for (i=1; i < respSize ; i++) 
    {
       delta = robotResp[i] - robotResp[i-1];
       // Save percentage
       weightRes[i] =  100 * (MAX_ANGLE_CONST - delta)/MAX_ANGLE_CONST;
       // Modify responses based on the weight and difference between them
       robotResp[i] *= weightRes[i]/100;
    } 
    // Response 0 will be linked to weight 1
    weightRes[0] = weightRes[1];
    robotResp[0] *= weightRes[1]/100;
    delta = 0;
    for (i = 0; i < respSize; i++)
      delta += (robotResp[i] - g_posP->angle) * weightRes[i]/(100*WEIGHT_GAIN_CONST);
    // Update new angle, delta contains new desired angle
    NORMALIZE_ANGLE(delta);
    desiredAngle = delta;
    pid_ret = pid(desiredAngle, g_posP->ang, ctx);  
    //Generate new tasks
    
}

ret_t processResponsePID(discPack_p pack)
{
    uint8_t i;
    if (isCTXInit = FALSE)
    {
       setPIDVals();
    }
    for (i=0; i < histSize; i++) 
    {
       delta = robotResp[i] - robotResp[i-1];
       // Save percentage
       weightRes[i] =  100 * (MAX_ANGLE_CONST - delta)/MAX_ANGLE_CONST;
       // Modify responses based on the weight and difference between them
       robotResp[i] *= weightRes[i]/100;
    } 
    // Response 0 will be linked to weight 1
    weightRes[0] = weightRes[1];
    robotResp[0] *= weightRes[1]/100;
    delta = 0;
    for (i = 0; i < respSize; i++)
      delta += (robotResp[i] - g_posP->angle) * weightRes[i]/(100*WEIGHT_GAIN_CONST);
    // Update new angle, delta contains new desired angle
    NORMALIZE_ANGLE(delta);
    desiredAngle = delta;
    pid_ret = pid(desiredAngle, g_posP->ang, ctx);  
    //Generate
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
      if (histSize < MAX_HIST_POS)
        histSize++;
    } 
}

/* Aux, init PID vals */
void setPIDVals();
{
     initPID(10,          // Kp
             5,           // Ki
             5,           // Kd
             300,         // Max overflow permitted
             &pidCtx);    // Context
     isCTXInit = TRUE;
}

void sortRes()
{
   // Mini sorting algorithm using insert sort
   uint8_t i,j;
   int16_t tmp;
   for (i=0; i < respSize; i++)
   {
      j = i;
      while ( j > 0 && robotResp[j] < robotResp[j - 1])
      {
         tmp = robotResp[j];
         robotResp[j] = robotResp[j - 1];
         robotResp[j - 1] = tmp;
         --j;
      }
   }
}
