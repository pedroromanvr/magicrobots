
#ifndef _PLATFORM_AVR_
#define _PLATFORM_AVR_

//Include chip specific library
#include <mega328p.h>

<<<<<<< HEAD
/* Valid return types for the movement module */
typedef enum {
    SUCCESS,                          
    WARNING, /* You should retry */
    FAIL, /* No reason, it just failed */
    ERROR, /* Something was printed to stdout */
    UNIMPLEMENTED
}
ret_t;
     
//1 if native SPI module included in micro, 
//0 if USI module 
=======
//1 if native SPI module included in micro,
//0 if USI module
>>>>>>> ec36d4710c01f2cab1f490c19751c7bb7f91d11d
#define NATIVE_SPI 1
#define TRUE    1
#define FALSE   0
#define GPIO_COMPLEMENT_BIT(x) \
<<<<<<< HEAD
    PORTC. ## x = PINC. ## x
#define EXEC_N_CHECK(ex, ret) \
do{                                                              \ 
    (ret) = (ex);                                                \
    if((ret) != SUCCESS && (ret) != WARNING)                     \
        printf("Error in expression, return code: %d\n", (ret)); \ 
}while(0)
=======
//    PORTC. ## x = PINC. ## x


#define TRUE    1
#define FALSE   0

#define DATA_FROM_PC  (UCSR0A & (1 << RXC0 ))
#define RX_BUF        UDR0
#define EOL_CHAR      0x0A


>>>>>>> ec36d4710c01f2cab1f490c19751c7bb7f91d11d

#endif
