/*
spi lib 0x01

copyright (c) Davide Gironi, 2012

References:
  -  This library is based upon SPI avr lib by Stefan Engelke
     http://www.tinkerer.eu/AVRLib/SPI

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef _SPI_H_
#define _SPI_H_

// Uncomment if your device has a real SPI compilant module
// Otherwise you will have to implement SPI somehow 
// #define NATIVE_SPI

#if defined(__GNUC__)
 #include <avr/io.h>
 #include <avr/interrupt.h>
#elif defined(__CODEVISIONAVR__)
    #include "../platform.h"
#else
	#error "Unsupported compiler"
#endif

#include <stdint.h>

//spi ports
// ############## Change for uC specific device ###############
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#if defined(__GNUC__)
 #define SPI_MISO PB4
 #define SPI_MOSI PB3
 #define SPI_SCK PB5
 #define SPI_SS PB2
#elif defined(__CODEVISIONAVR__)
 #define SPI_MISO PORTB4
 #define SPI_MOSI PORTB3
 #define SPI_SCK PORTB5
 #define SPI_SS PORTB2
#endif

extern void spi_init();
extern uint8_t spi_writereadbyte(uint8_t data);
#endif
