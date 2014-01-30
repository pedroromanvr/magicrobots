
#include "random.h"

uint16_t randomIdx = 0;

uint16_t random16()
{
    uint16_t ret = random8();
    ret |= (((uint16_t)random8())<<8);
    return ret;
}
