
#ifndef __MOVEMENT_MODULE__
#define __MOVEMENT_MODULE__

#define FRONT_PIN   PORTC.0
#define BACK_PIN    PORTC.1
#define RIGHT_PIN   PORTC.2
#define LEFT_PIN    PORTC.3

#define PWM_FRECUENCY_MAX 10
#define PWM_FRECUENCY_CNT 3
#define MAX_LIST_SIZE 100

/* Valid commands that can be called remotely */
typedef enum {
    UNUSED, 
    FRONT,
    BACK,    
    RIGHT,
    LEFT,
    ROTATE_RIGHT,
    ROTATE_LEFT,
    ROTATE_RIGHT_INVERTED,
    ROTATE_LEFT_INVERTED,
    MOVE_MAX
} 
moveCmd_t;

/* Valid return types for the movement module */
typedef enum {
    SUCCESS,                          
    WARNING, /* You should retry */
    FAIL, /* No reason, it just failed */
    ERROR, /* Something was printed to stdout */
    UNIMPLEMENTED
}
ret_t;

/* Status updates for the movement process */
typedef enum {
    DONE,
    STARTED,
    PROGRESS, 
    PENDING
}
state_t;

/* current movement process descriptor */
typedef struct {
    moveCmd_t command;
    state_t   state;    
    char       importance; /* A more important command can cancel the current one */
    void     *next; 
}
movementState_t;
typedef movementState_t * movementState_p;

/* Current executed movement */
typedef struct {
   unsigned char curVal;
   unsigned char isCompleted;    
   movementState_p curState;
}
curMovement_t;

/* global movement descriptor */
typedef struct {
    movementState_p head;
    movementState_p tail;
    char requestPending; /* Number of movements pending */    
    char inUse; /* honor mutex 0=available */
}
globalState_t;
typedef globalState_t * globalState_p;

/* list manipulation functions (FIFO style) */
ret_t addMovement(movementState_p movement);
ret_t queryMovement(movementState_p *movement);
ret_t purgeMovementList();

/* movement functions */
/* extern ret_t (*movementFuncArr[MOVE_MAX])(void *args) */
ret_t unusedFunc(void *args); /* This function should detect errors */
ret_t frontFunc(void *args);
ret_t backFunc(void *args);
ret_t rightFunc(void *args);
ret_t leftFunc(void *args);
ret_t rotateRightFunc(void *args);
ret_t rotateLeftFunc(void *args);
ret_t rotateRightInvertedFunc(void *args);
ret_t rotateLeftInvertedFunc(void *args);

/* Variables to be used 
Array of function pointers to execute the remote command */
extern ret_t (*movementFuncArr[MOVE_MAX])(void *args);
extern globalState_p globalP;
//curMovemnt to be used outside this file
/*  curVal contains the bit field used to define the current movement
    Bit description:
        * Bit 0 : isFront bit 
        * Bit 1 : isBack bit
        * Bit 2 : isRight bit
        * Bit 3 : isLeft bit     
        * Bit 4-7 : N/A
*/
extern curMovement_t curMovement;

/* Entry point for the movement module
 * This functions should be the only ones called from the outside
 * It should do all validity checks that are needed for the commands
 * and it should execute the command.
 * The return type is not specified.
 */
ret_t movementCommandExecute(moveCmd_t command);
ret_t workerMovement();
ret_t validateMovement();

#define lockGlobalState() \
do{ \
    if(globalP->inUse) \
        return WARNING; \
    else \             
        globalP->inUse = 1; \
}while(0)

#define releaseGlobalState() \
do{ \
    if(globalP->inUse) \
        globalP->inUse = 0; \
    else \             
        return FAIL; \
}while(0)

#endif /* __MOVEMENT_MODULE__ */