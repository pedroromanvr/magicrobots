/*******************************************************
Test 1: Simple rootID validations to aux functions in network.c

*******************************************************/

#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "../network/network.h"

void initIO();

void main(void)
{
  /* ############## Local variables ############## */ 
  ripEntry_t re;
  uint8_t address;
  uint16_t rootID;
  uint16_t leafID;

  int i;         
  /* ############################################# */
  initIO();
  
  printf("Starting test\n");

  /* Test isRootId */
  rootID = 65;
  printf("isRootID(%d) Result: %d\n", rootID, isRootId(rootID)); 
  rootID = 60;
  printf("isRootID(%d) Result: %d\n", rootID, isRootId(rootID)); 
  rootID = 61;
  printf("isRootID(%d) Result: %d\n", rootID, isRootId(rootID)); 
  rootID = 63;
  printf("isRootID(%d) Result: %d\n", rootID, isRootId(rootID)); 
  rootID = 200;
  printf("isRootID(%d) Result: %d\n", rootID, isRootId(rootID)); 

  /* Test isInRange */
  leafID = 67;
  rootID = 65;
  printf("isInRange(%d,%d) Result: %d\n", leafID, rootID, 
                                isInRange(leafID, rootID)); 
  leafID = 213;
  rootID = 217;
  printf("isInRange(%d,%d) Result: %d\n", leafID, rootID, 
                                isInRange(leafID, rootID)); 
  leafID = 654;
  rootID = 655;
  printf("isInRange(%d,%d) Result: %d\n", leafID, rootID, 
                                isInRange(leafID, rootID)); 
  leafID = 614;
  rootID = 610;
  printf("isInRange(%d,%d) Result: %d\n", leafID, rootID, 
                                isInRange(leafID, rootID)); 

  /* Test getRootFromID */
  leafID = 64;
  printf("getRootFromID(%d) Result: %d\n", leafID, getRootFromID(leafID)); 
  leafID = 1113;
  printf("getRootFromID(%d) Result: %d\n", leafID, getRootFromID(leafID)); 
  leafID = 267;
  printf("getRootFromID(%d) Result: %d\n", leafID, getRootFromID(leafID)); 
  leafID = 11;
  printf("getRootFromID(%d) Result: %d\n", leafID, getRootFromID(leafID)); 
  leafID = 56;
  printf("getRootFromID(%d) Result: %d\n", leafID, getRootFromID(leafID)); 

  /* Test for insertEntry */
  re.address = 51;
  re.isRoot = 0;
  re.id = 1000;

  for(i=0; i<(_MAX_PIPES_-1); i++)
  {
     insertEntry(&re);
     re.address++;
     re.id++;  
  }
  //Insert more than 5
  insertEntry(&re);

  /* Test for IsInRIP */
  address = 56;
  printf("isInRIP(%d) Result: %d\n", address, isInRIP(address)); 
  address = 51;
  printf("isInRIP(%d) Result: %d\n", address, isInRIP(address)); 
  address = 200;
  printf("isInRIP(%d) Result: %d\n", address, isInRIP(address)); 
  address = 53;
  printf("isInRIP(%d) Result: %d\n", address, isInRIP(address)); 

  printf("Testing done,\n");
  while(1);
}  

void initIO()
{
    // Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTB=0x00;
DDRB=0x00;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Normal top=0xFF
// OC0A output: Disconnected
// OC0B output: Disconnected
TCCR0A=0x00;
TCCR0B=0x00;
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=0x00;
TCCR2A=0x00;
TCCR2B=0x00;
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: Off
EICRA=0x00;
EIMSK=0x00;
PCICR=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x00;

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=0x00;

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=0x00;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART0 Mode: Asynchronous
// USART Baud Rate: 9600
UCSR0A=0x00;
UCSR0B=0x18;
UCSR0C=0x06;
UBRR0H=0x00;
UBRR0L=0x33;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
ADCSRB=0x00;
DIDR1=0x00;

// ADC initialization
// ADC disabled
ADCSRA=0x00;

// SPI initialization
// SPI disabled
SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;    
}
