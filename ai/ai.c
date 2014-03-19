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
    uint8_t i, rand;
    int16_t delta, pid_ret;
    ret_t ret;
    if (isCTXInit == FALSE)
    {
       setPIDVals();
    }
    aiDebugPrint("processMyPID: Processing my PID\n");
    if (respSize > 0)
    {
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
      delta = NORMALIZE_ANGLE(delta);
      desiredAngle = delta;
    }
    // Randomize our path to search
    rand = random8() % MAGIC_FACTOR;
    desiredAngle += rand & 0x01 ? rand : -1 * rand;
    pid_ret = pid(desiredAngle, g_posP->angle, &pidCtx);  
    
    //Generate new tasks
    if (pid_ret < 0) // Go Left
    {
       EXEC_N_CHECK(movementCommandExecute1(ROTATE_LEFT, TEXEC_CONST*pid_ret/MAX_VAL_RET), ret);
    }
    else  //Go right
    {
       EXEC_N_CHECK(movementCommandExecute1(ROTATE_RIGHT, TEXEC_CONST*pid_ret/MAX_VAL_RET), ret);
    }        
    aiDebugPrint("processMyPID: Success, got pid val: %d\n", pid_ret);
    return SUCCESS;
}

ret_t processResponsePID(discPack_p pack)
{
    uint8_t i;
    int16_t sum = 0, pidret;
    headerPack_t head;
    locationRequest_t lr;
    pos_p pp = (pos_p)pack->data;
    if (isCTXInit == FALSE)
    {
       setPIDVals();
    }
    aiDebugPrint("processResPID: processing Response from %d\n", (pack->header).idSrc);
    if (histSize > 0)
    {
      for (i=0; i < histSize; i++) 
      {
         sum += historicalPos[i].angle;
      }
      // Normalize angle
      sum = NORMALIZE_ANGLE(sum);
      // Return a perpendicular line between our movement and the other robot
      pidret = MAGIC_FACTOR * pidPeek(pp->angle, sum, &pidCtx) / MAX_VAL_RET;
      pidret = NORMALIZE_ANGLE(pidret);
      // Construct an answer
      head.type = RAW;
      head.idDest = (pack->header).idSrc;
      lr.type = ADVICE_RESPONSE;
      lr.position.angle = pidret;
      sendMessageTo(&head, (char *)&lr, sizeof(locationRequest_t));
      aiDebugPrint("processResPID: Sent response %d OK\n", pidret);
    }
    return SUCCESS;
}

ret_t sendPosMulticast(void)
{
    locationRequest_t req;
    req.type = ADVICE_REQUEST;
    if(isValidPos == FALSE)
        return WARNING;
    respSize = 0;
    memcpy(&(req.position), g_posP, sizeof(pos_t));
    aiDebugPrint("sendPosMulticast: Broadcasting current location\n");
    return sendMessage((char *)&req, sizeof(locationRequest_t));
}

uint8_t getResponse(void)
{
    ret_t ret;             
    discPack_t packet;
    locationRequest_p lr;    
    ret = networkManager(&(packet.header), packet.data, DATA_SIZE);
    if(ret == WARNING)
    {                 
        lr = (locationRequest_p)packet.data;
        if(lr->type == ADVICE_RESPONSE)
        {       
            aiDebugPrint("getResponse: got ADVICE_RESPONSE\n");
            robotResp[respSize % MAX_NO_RESPONSES] = lr->position.angle;
            respSize++;
        }
    }
    return respSize;
}

ret_t getRequest(discPack_p packet)
{
    ret_t ret;
    locationRequest_p lr;    
    ret = networkManager(&(packet->header), packet->data, DATA_SIZE);
    if(ret == WARNING)
    {                 
        lr = (locationRequest_p)packet->data;
        if(lr->type == ADVICE_REQUEST)
        {
            aiDebugPrint("getRequest: got ADVICE_REQUEST\n");
            return SUCCESS;
        }
    }
    return WARNING;
}

extern ret_t readPosition()
{
    locationRequest_t loc;
    ret_t ret;
    
    aiDebugPrint("readPosition: Calling getLocation\n");
    isValidPos = FALSE;
    ret = getLocation(&loc);
    if(ret == WARNING && loc.type == RESPONSE)
    {
        aiDebugPrint("readPosition: Valid location found\n");
        memcpy(&g_pos, &(loc.position), sizeof(pos_t));
        isValidPos = TRUE;
        return SUCCESS;
    }
    aiDebugPrint("readPosition: Can't provide valid position\n");
    return WARNING;
}

ret_t savePosition(void)
{
    static uint8_t ix = 0;
    if (isValidPos) {
      memcpy(&historicalPos[ix], g_posP, sizeof(pos_t));
      ix = (ix + 1) % MAX_HIST_POS;
      if (histSize < MAX_HIST_POS)
        histSize++;
      aiDebugPrint("savePosition: Saved valid position\n");
    }         
    return SUCCESS;
}

/* Aux, init PID vals */
void setPIDVals()
{
     initPID(10,          // Kp
             5,           // Ki
             5,           // Kd
             300,         // Max overflow permitted
             &pidCtx);    // Context
     isCTXInit = TRUE;
}

void sortResp()
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
