#include "network.h"

ripEntry_t ripTable[_MAX_PIPES_] = {{0,0},
                                    {0,0},
                                    {0,0},
                                    {0,0},
                                    {0,0}};
uint8_t usedEntries = 0;
uint16_t gID = 0;  

#include "platform.h"
#include "stdlib.h"
#include <delay.h>

#define _JOIN_PIPE_ 0
#define _MAX_RETRIES_ 100
#define _DELTA_DELAY_ 10
// Voltage Reference: AREF pin
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))


//To use outside this API
ret_t joinNetwork()
{
    // ----------- Variable declarations ----------
    uint8_t pipe;   
    char data[NRF24L01_PAYLOAD];   
    ripEntry_t *entryP = (ripEntry_t *)data;    
    uint8_t retryN = 0;
                                                   
    // --------------------------------------------
    srand(TCNT2);

    gID = rand(); 
    printf("gID %s:%d:%d\n", __FILE__, __LINE__, gID);   
    
    // Select initial pipe, range 1-255
    pipe = gID%255 + 1;
    // Validate root
    if(isRootPipe(pipe))
    {  
        while(retryN < _MAX_RETRIES_)
        {
            //Wait for a non-root node in range to send a message
            while( !nrf24l01_readready(_JOIN_PIPE_) );
            //Someone sends us a message  
            nrf24l01_read(data);
            if(isInRange(entryP->pipe, pipe)) 
            {
                if(usedEntries<_MAX_PIPES_)
                {
                    insertEntry(entryP);
                }  
                else                           
                {
                    //Convert leaf node to root 
                    //break outter while
                }
            }
            else         
            {
                retryN++;
            }
        }                              
    }
    else // Leaf node
    {
        //Look for our root to become available
    }                                                             
    return SUCCESS;
}
//Broadcast message
 ret_t sendMessage(char *msg,  uint16_t size){
    return UNIMPLEMENTED;
}
//Send message to an specific ID
 ret_t sendMessageTo(uint16_t id, char *msg, uint16_t size){
    return UNIMPLEMENTED;
}
//Recieve message from any sender
 ret_t getMessage(char *buf, uint16_t size){
    return UNIMPLEMENTED;
}
//Specify ID to recieve from
 ret_t getMessageFrom(uint16_t id, char *buf, uint16_t size){
    return UNIMPLEMENTED;
}

ret_t insertEntry(ripEntry_t *newEntry)
{
    return UNIMPLEMENTED;
}

/* Internal, aux functions */
int isRootPipe(uint16_t pipe)
{
    return 0;
}

isInRange(uint16_t leafPipe, uint16_t rootPipe)
{
    return 0;    
}

ret_t initADC()
{
    
    // ADC initialization
    // ADC Clock frequency: 1000.000 kHz
    // ADC Voltage Reference: AREF pin
    // ADC Auto Trigger Source: ADC Stopped
    // Digital input buffers on ADC0: Off, ADC1: On, ADC2: On, ADC3: On
    // ADC4: On, ADC5: On
    DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (1<<ADC0D);
    ADMUX=ADC_VREF_TYPE;
    ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
}


ret_t disableADC()
{
    // ADC initialization
    // ADC disabled
    ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
    return SUCCESS;
}

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

