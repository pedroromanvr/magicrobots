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
    SERVICE_UNAVAILABLE
}locReqType_t;
typedef struct
{
    locReqType_t type;
    uint8_t x;
    uint8_t y;
}locationRequest_t;
typedef locationRequest_t* locationRequest_p;

ret_t getLocation(locationRequest_p lr);

#define locatorDebugPrint printf