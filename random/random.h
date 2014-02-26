#ifndef __RANDOM__
#define __RANDOM__

#include <stdint.h>
#include "../platform.h"

/*
 * You should place a random sequence in the eeprom before hand.
 * CodeVisionAVR will use the required functions to read from the EEPROM
 */
extern eeprom uint8_t numbers[1024];
extern uint16_t randomIdx;

#define random8() \
    (numbers[(randomIdx++)%1024])

uint16_t random16();

#endif