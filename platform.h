
#ifndef _PLATFORM_AVR_
#define _PLATFORM_AVR_

//Include chip specific library
#include <mega328p.h>

//1 if native SPI module included in micro,
//0 if USI module
#define NATIVE_SPI 1
#define GPIO_COMPLEMENT_BIT(x) \
//    PORTC. ## x = PINC. ## x


#define TRUE    1
#define FALSE   0

#define DATA_FROM_PC  (UCSR0A & (1 << RXC0 ))
#define RX_BUF        UDR0
#define EOL_CHAR      0x0A



#endif
