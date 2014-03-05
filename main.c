/*******************************************************

This program was created by the
CodeWizardAVR V3.08 Evaluation
Automatic Program Generator
� Copyright 1998-2013 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 07/12/2013
Author  : 
Company : 
Comments: 

Chip type               : ATmega328P
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512

*******************************************************/

#include "platform.h"
#include "nrf24l01/nrf24l01.h"
//#include "movement/movement.h"
#include "movement/stateMachine.h"
#include "network/network.h"
#include "chat/chat.h"
#include "dongle/dongle.h"
#include <delay.h>
#include <string.h>

// Declare your global variables here
// Standard Input/Output functions
#include <stdio.h>

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
  // Reinitialize Timer 0 value                
  TCNT0=0xF3;
  // Manage timer constants
  timerVector[TBASE]++;
  timerVector[TPWM]++;                    // Used in the state machine
  if (timerVector[TBASE] == TBASE_CONST)  // Each 1ms
  {
    timerVector[TBASE] = 0;
    timerVector[TSTART]++;
    timerVector[TEXEC]++;    
    timerVector[TSAVE_DATA]++;
  }
}

void setupHardware()
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
// Func7=Out Func6=In Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=0 State6=T State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
PORTB=0x00;
DDRB=0xBF;

// Port C initialization
// Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State6=0 State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
PORTC=0x00;
DDRC=0x7F;

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

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
ADCSRB=0x00;
DIDR1=0x00;

// ADC initialization
// ADC disabled
ADCSRA=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;    
}

void main_for_chat(void)
//void main(void)
{
    setupHardware();
    initChat();
    INIT_NW_STACK();
    printf("NW stack inited\n");
    // Global enable interrupts
    #asm("sei")
    if(enterMicroRoom() == SUCCESS)
        printf("Thanks for using chatroom!\n");
    else
        printf("chatroom service terminated abnormally!\n");
        
    return;
}
//void main_for_dongle(void)
void main(void)
{
    setupHardware();
    dongleInit();
    // Global enable interrupts
    #asm("sei")
    if(dongleMainThread() == SUCCESS)
        printf("DEBUG=Thanks for using dongle!\n");
    else
        printf("DEBUG=dongle service terminated abnormally!\n");
        
    return;
}
