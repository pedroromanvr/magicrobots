#include "locator.h"

ret_t getLocation(locationRequest_p lr)
{
    ret_t ret;
    headerPack_t header;
    discPack_t packet;

    if(!isPaired && !isRootId(gID))
    {
        netDebugPrint("DEBUG=getLocation: calling joinNetworkOnTheFly for leaf\n");
        joinNetworkOnTheFly(NULL);
        if(!isPaired)
            return SUCCESS;
    }

    lr->type = REQUEST;
    lr->position.x = 0;
    lr->position.y = 0;
    lr->position.angle = 0;
    
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

    ret = networkManager(&header, packet.data, DATA_SIZE);
    if(ret == WARNING && header.idSrc == LOCATION_SERVICE_GID)
    {
        memcpy(lr, packet.data, sizeof(locationRequest_t));
        return WARNING;
    }

    locatorDebugPrint("DEBUG=getLocation: Timed out\n");
    lr->type = SERVICE_UNAVAILABLE;
    lr->position.x = 0;
    lr->position.y = 0;
    lr->position.angle = 0;
    return SUCCESS;
}