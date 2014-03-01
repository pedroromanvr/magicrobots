
#ifndef _PLATFORM_AVR_
#define _PLATFORM_AVR_

//Include chip specific library
#include <mega328p.h>

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
#define NATIVE_SPI 1
#define TRUE    1
#define FALSE   0
#define GPIO_COMPLEMENT_BIT(x) \
    PORTC. ## x = PINC. ## x
#define EXEC_N_CHECK(ex, ret) \
do{                                                              \ 
    (ret) = (ex);                                                \
    if((ret) != SUCCESS && (ret) != WARNING)                     \
        printf("Error in expression, return code: %d\n", (ret)); \ 
}while(0)

#endif
