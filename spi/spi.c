/*
spi lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include "spi.h"
/*
 * spi initialize
 */
void spi_init() {
    SPI_DDR &= ~((1<<SPI_MOSI) | (1<<SPI_MISO) | (1<<SPI_SS) | (1<<SPI_SCK)); //input
    SPI_DDR |= ((1<<SPI_MOSI) | (1<<SPI_SS) | (1<<SPI_SCK)); //output

#if (NATIVE_SPI==1)
//SPI initialization

// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 2*500,000 kHz
// SPI Clock Phase: Cycle Start
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
SPCR=0x51;
SPSR=0x01;

#else            
// USI initialization
// Mode: Three Wire (SPI)
// Clock source: Register & Counter=no clk.
// USI Counter Overflow Interrupt: Off                                               
USICR=(0<<USISIE) | (0<<USIOIE) | (0<<USIWM1) | (1<<USIWM0) | (0<<USICS1) | (0<<USICS0) | (0<<USICLK) | (0<<USITC);
#endif
}

/*
 * spi write one byte and read it back
 */
#if (NATIVE_SPI==1)
uint8_t spi_writereadbyte(uint8_t data) {
    SPDR = data;
    while((SPSR & (1<<SPIF)) == 0);
    return SPDR;       
}
#else
uint8_t spi_writereadbyte(uint8_t data) {
    USIDR = data;
    while((USISR & (1<<USIOIF)) == 0);
    return USIDR;
}
#endif



