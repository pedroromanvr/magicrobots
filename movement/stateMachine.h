#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "../platform.h"
#include "../ai/ai.h"
#include "../network/network.h"
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
    FRONT0,       //  PWM movement
    FRONT1,
    FRONT2,
    BACK0,
    BACK1,
    BACK2,
    SEND_POS,
    WAIT_RES,
    CALC_PID,
    WAIT_REQ,
    CALC_RES,
    STATES_MAX
} 
states_t;

enum timerVectorEnum {
    TBASE,
    TSTART,
    TPWM,
    TEXEC,
    TSAVE_DATA,
    TRESP_TIMEOUT,
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

// Timer0 vector
extern uint16_t timerVector[TSIZE];
extern uint8_t g_pwmDuty;

#define smDebug printf  

/* ---- CONSTANTS ---- */
#define TBASE_CONST       10
#define TSTART_CONST      100
#define TPWM_CONST        3 
#define TPWM_MAX          10
#define TRES_TOUT_CONST   100
#define MIN_RESP_CONST    3
#define TSAVE_DATA_CONST  5000

/* ---- INPUTS ---- */
#define INIT_FLAG         (workerMovement() == SUCCESS)
#define GO_FWD            1
#define GO_RIGHT          goRight
#define GO_LEFT           goLeft
#define DUTY_REACHED      (timerVector[TPWM] >= g_pwmDuty)
#define FREQ_REACHED      (timerVector[TPWM] >= TPWM_MAX)
#define TIMER_START_DONE  (timerVector[TSTART] >= TSTART_CONST)
#define TIMER_EXEC_MAX    (timerVector[TEXEC] >= g_timExec) 
#define TIMER_RSP_TIMEOUT (timerVector[TRESP_TIMEOUT] >= TRES_TOUT_CONST)
#define T_SAVE_DATA       (timerVector[TSAVE_DATA] >= TSAVE_DATA_CONST)

/* ---- OUTPUTS ---- */
#define FRONT_PIN   PORTC.0
#define BACK_PIN    PORTC.1
#define RIGHT_PIN   PORTC.2
#define LEFT_PIN    PORTC.3

#endif /* __STATE_MACHINE__*/
