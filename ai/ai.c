#include "ai.h"
#include <string.h>

/* Global variables */
pos_t g_pos = {255, 255, -500};      // Initialize with invalid values
pos_p g_posP = &g_pos;
pos_t historicalPos[MAX_HIST_POS];   // Used to save historical positions
int16_t robotResp[MAX_NO_RESPONSES]; // Save responses of all robots 
uint8_t isValidPos = FALSE;
uint8_t respSize = 0; 

ret_t processMyPID(void)
{
    
}

ret_t processResponsePID(void)
{
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
    } 
}
