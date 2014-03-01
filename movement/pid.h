#ifndef __PID__
#define __PID__

#include <stdint.h>

typedef struct {
  uint16_t Kp;            // Proportional gain
  uint16_t Ki;            // Integral gain
  uint16_t Kd;            // Derivativegain
  int16_t errorSum;       // Used to save historical error 
  int16_t lastError;      // Used to save the last error 
  uint16_t overflowVal;   // Max allowed value in error
} 
pidCtx_t;

typedef pidCtx_t * pidCtx_p;

/*************************************************/
/*    Functions to be called outside this API    */
/*************************************************/

/*  Inits the context of the PID controller
 *  Tunning of this PID controller can be achived using Ziegler-Nichols 
 *  parameters.
 *  Kp       Ki       Kd 
 *  0.65*Kc  2/Pc   0.0833/Pc 
 *    where:
 *      Kc is the critical gain
 *      Pc is the critical Period
 *
 *  @param Kp is the proportional gain
 *  @param Ki is the integral gain
 *  @param Kd is the derivative gain
 *  @param overflow is the error maximum permitted value
 *  @param ctx is the context to initialize
 */
extern void initPID(uint16_t Kp, uint16_t Ki, uint16_t Kd, 
                    uint16_t overflow, pidCtx_p ctx);

/*  Returns the output of the controller using a discrete PID algorithm.
 *
 *  @param setPoint the desired value
 *  @param procVal  the real value got from feedback
 *  @param ctx      context for this PID controller
 *
 *  @return an int16 with the PID value to apply to the system
 */
extern int16_t pid(int16_t setPoint, int16_t procVal, pidCtx_p ctx);

/**************************************************/

#endif /* __PID__*/
