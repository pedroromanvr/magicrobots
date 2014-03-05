#include "locator.h"

ret_t getLocation(locationRequest_p lr)
{
    uint32_t i = 0;   
    ret_t ret;
    headerPack_t header;
    discPack_t packet;
    
    lr->type = REQUEST;
    lr->x = 0;
    lr->y = 0;
    
    header.type = RAW;
    header.size = sizeof(locationRequest_t);
    header.ttl = DEFAULT_TTL;
    header.number = 0;
    header.idSrc = gID;
    header.idDest = LOCATION_SERVICE_GID;
    header.checksum = checksumCalculator(&header, (char *)lr, sizeof(locationRequest_t));
    
    locatorDebugPrint("DEBUG=getLocation: Calling microSendMessage\n");
    microSendMessage(&header, (char *)lr, sizeof(locationRequest_t));

    locatorDebugPrint("DEBUG=getLocation: Waiting for answer...\n");
    while(i < DEFAULT_TIMEOUT)
    {
        i++;
        ret = networkManager((char *)(&packet), sizeof(discPack_t));
        if(ret == SUCCESS)
            continue;
        if(ret == WARNING)
        {
            memcpy(lr, packet.data, sizeof(locationRequest_t));
            return WARNING;
        }
    }
    locatorDebugPrint("DEBUG=getLocation: Timed out\n");
    lr->type = SERVICE_UNAVAILABLE;
    lr->x = 0;
    lr->y = 0;
    return SUCCESS;
}