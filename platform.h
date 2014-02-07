
#ifndef _PLATFORM_AVR_
#define _PLATFORM_AVR_

//Include chip specific library
#include <mega328p.h>
      
//1 if native SPI module included in micro, 
//0 if USI module 
#define NATIVE_SPI 1
#define GPIO_COMPLEMENT_BIT(x) \
    PORTC. ## x = PINC. ## x
#endif
