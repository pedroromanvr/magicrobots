#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "../platform.h"
#include "movement.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Possible states this machine will have */
typedef enum {
    INVALID_S, 
    IDLE,
    INIT_MOV0,    // These states will be used to break stop-motion inertia
    INIT_MOV1,
    INIT_MOV2,
    INIT_MOV3,
    INIT_MOV4,
    FRONT0,       //  PWM movement
    FRONT1,
    FRONT2,
    FRONT3,
    FRONT4,
    BACK0,
    BACK1,
    BACK2,
    BACK3,
    BACK4,
    STATES_MAX
} 
states_t;

enum timerVectorEnum {
    TBASE,
    TSTART,
    TPWM,
    TEXEC,
    TSAVE_DATA,
    TSIZE
};

/**** Functions to be called outside this API ****/

/*  Inits this state machine 
 *  @param none
 */
extern ret_t initMachine(void);

/*  Function to be polled outside to make the state machine work
 *  @param none
 */
extern ret_t processMachine(void);

/*  Function to set the Duty cycle, default value is 3 (30%) 
 *  @param val is the duty cycle to use range (0-10) 
 */
extern ret_t setPWMDuty(uint8_t val);

/**************************************************/

/* Auxiliary functions */
ret_t processOutputs();
void initTimer();
void stopTimer();
uint16_t calculatePID();

// Timer0 vector
extern uint16_t timerVector[TSIZE];
extern uint8_t g_pwmDuty;

#define ANGLE_ADD(ang)    \
  

/* ---- CONSTANTS ---- */
#define TBASE_CONST   10
#define TSTART_CONST  100
#define TPWM_CONST    3 
#define TPWM_MAX      10 
#define TEXEC_CONST   2000

/* ---- INPUTS ---- */
#define INIT_FLAG         1
#define GO_FWD            1
#define GO_RIGHT          1
#define GO_LEFT           0
#define DUTY_REACHED      (timerVector[TPWM] >= g_pwmDuty)
#define FREQ_REACHED      (timerVector[TPWM] >= TPWM_MAX)
#define TIMER_START_DONE  (timerVector[TSTART] >= TSTART_CONST)
#define TIMER_EXEC_MAX    (timerVector[TEXEC] >= TEXEC_CONST) 

/* ---- OUTPUTS ---- */
#define FRONT_PIN   PORTC.0
#define BACK_PIN    PORTC.1
#define RIGHT_PIN   PORTC.2
#define LEFT_PIN    PORTC.3

#endif /* __STATE_MACHINE__*/
