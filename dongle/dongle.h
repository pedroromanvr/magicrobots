#ifndef __DONGLE__
#define __DONGLE__

#include "../network/network.h"
#include "../movement/movement.h"
#include "../locator/locator.h"
#include "../platform.h"

#include <stdint.h>
#include <stdio.h>

ret_t dongleInit();
ret_t dongleMainThread();

#define dongleDebugPrint printf

#endif
