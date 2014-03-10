#ifndef __LOCATOR__
#define __LOCATOR__

#include "../network/network.h"
#include "../movement/movement.h"
#include "../platform.h"

#include <stdint.h>

#define LOCATION_SERVICE_GID 240

typedef enum
{
    UNUSED_LRT,
    REQUEST,
    RESPONSE,          
    ADVICE_REQUEST,
    ADVICE_RESPONSE,
    SERVICE_UNAVAILABLE
}locReqType_t;
typedef struct
{
    locReqType_t type;
    pos_t    position;
}locationRequest_t;
typedef locationRequest_t* locationRequest_p;

ret_t getLocation(locationRequest_p lr);

#define locatorDebugPrint printf

#endif /* __LOCATOR__ */