
#ifndef __MOVEMENT_MODULE__
#define __MOVEMENT_MODULE__

#include "../platform.h"
#include <stdint.h>

#define MAX_LIST_SIZE 10
#define DEFAULT_TIME  2000

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

/* Status updates for the movement process */
typedef enum {
    DONE,
    PENDING
}
state_t;

/* movement process descriptor */
typedef struct {
    moveCmd_t command;
    state_t   state;    
    uint16_t  time;   //Execution time of this state
}
movementState_t;
typedef movementState_t * movementState_p;

/* global movement descriptor */
typedef struct {
    uint8_t head;
    uint8_t tail;
    uint8_t size;           /* Number of movements pending */    
    uint8_t inUse;          /* honor mutex 0=available */
}
globalState_t;
typedef globalState_t * globalState_p;

/* list manipulation functions (FIFO style) */
ret_t addMovement(moveCmd_t movement, uint16_t time);
ret_t queryMovement(movementState_p *movement);
ret_t getMovement(movementState_p *movement);

/* Variables to be used */
extern movementState_t g_movements[MAX_LIST_SIZE];
extern globalState_p globalP;
extern movementState_t curMov;

/* Entry point for the movement module
 * This functions should be the only ones called from the outside
 * It should do all validity checks that are needed for the commands
 * and it should execute the command.
 * The return type is not specified.
 */

/* 
 * Function to queue a movement to execute.
 *
 * @param command is the command to queue
 * @return ERROR is the queue es full, chech STDOUT
 */
ret_t movementCommandExecute(moveCmd_t command);

/*
 * Queue movement with time
 *
 * @param time is the number of ms to exectue this command
 */
ret_t movementCommandExecute1(moveCmd_t command, uint16_t time);

/* 
 * Function to dequeue a movement to execute. This function will write on
 * curMov the next command to execute.
 *
 * @param command is the command to queue
 * @return ERROR if there is nothing to execture, check STDOUT
 */
ret_t workerMovement();

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

#define NEXT_INDEX(i)  \
(i) = ((i) + 1) % MAX_LIST_SIZE

#endif /* __MOVEMENT_MODULE__ */
