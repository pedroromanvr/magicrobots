#include "pid.h"
#include <stdlib.h> // abs()

void initPID(uint16_t Kp, uint16_t Ki, uint16_t Kd, 
                    uint16_t overflow, pidCtx_p ctx)
{
  ctx->Kp          = Kp;
  ctx->Ki          = Ki;
  ctx->Kd          = Kd;
  ctx->errorSum    = (int16_t)0;
  ctx->lastError   = (int16_t)0;
  ctx->overflowVal = overflow;
}

int16_t pid(int16_t setPoint, int16_t procVal, pidCtx_p ctx)
{
    int16_t curError, pidP, pidI, pidD;
    // Calculate errors
    curError = setPoint - procVal;
    if (abs(curError) > ctx->overflowVal)
          curError = curError > 0 ? ctx->overflowVal : -ctx->overflowVal;
    
    ctx->errorSum += curError;
    if (abs(ctx->errorSum) > ctx->overflowVal)
          ctx->errorSum = ctx->errorSum > 0 ? ctx->overflowVal : -ctx->overflowVal;
    // PID values
    pidP = ctx->Kp * curError;
    pidI = ctx->Ki * ctx->errorSum;
    pidD = ctx->Kd * (curError - ctx->lastError);

    ctx->lastError = curError;  // Save last error

    return (int16_t)(pidP + pidI + pidD);
}
