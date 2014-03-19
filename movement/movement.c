
#include "movement.h"
#include "../platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

movementState_t g_movements[MAX_LIST_SIZE];
globalState_t global = {0, 0, 0, 0};
globalState_p globalP = &global;
movementState_t curMov;

ret_t movementCommandExecute(moveCmd_t command)
{
   return movementCommandExecute1(command, DEFAULT_TIME);
}
ret_t movementCommandExecute1(moveCmd_t command, uint16_t time)
{
    ret_t ret;
    
    if(!globalP)
        goto failXit;
    // Add element to queue
    while ((ret = addMovement(command, time)) == WARNING);
    if(ret != SUCCESS)
        goto failXit;

    return SUCCESS;    
    failXit:
    printf("%s:%d:command=%d\nretCode=%d\n", 
        __FILE__, __LINE__, command, ret);
    return ERROR;
}
/* workerMovement must be called in a high privilege
   environment.
   It should not be interrupted during it's execution.
 */
ret_t workerMovement()
{
    movementState_p current = NULL;
    char retCode = -1;

    while((retCode = getMovement(&current)) == WARNING);
    if(retCode != SUCCESS)
        goto failXit;
        
    if(current)
    {
        // Set current movement
        memcpy(&curMov, current, sizeof(movementState_t)); 
    }   
    return SUCCESS;    

failXit:
    printf("%s:%d:current=%x\nretCode%d\n", 
        __FILE__, __LINE__, current, retCode);    
    return ERROR;
}

ret_t addMovement(moveCmd_t movement, uint16_t time)
{
    if(!globalP || movement >= MOVE_MAX)
        goto failXit;
    if( !(movement > UNUSED && movement < MOVE_MAX) )
    {
       printf("Error, movement not available\n");
       goto failXit;
    }

    lockGlobalState();
    
    if(globalP->size >= MAX_LIST_SIZE) // List is full
    {
        releaseGlobalState();
        printf("List is full\n");
        goto failXit;          
    } 
    
    // Add element to tail    
    g_movements[globalP->tail].command = movement;
    g_movements[globalP->tail].state   = PENDING;
    g_movements[globalP->tail].time    = time;

    globalP->size++;
    NEXT_INDEX(globalP->tail);

    releaseGlobalState();    
    
    return SUCCESS;

failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail, movement);    
    return ERROR;
}

ret_t queryMovement(movementState_p *movement)
{
    if(!globalP || !movement)
        goto failXit;

    lockGlobalState();

    if(globalP->size == 0)
    {
        releaseGlobalState();
        printf("Empty list\n");
        goto failXit;          
    }
    *movement = &g_movements[globalP->head];
    
    releaseGlobalState();

    return SUCCESS;
failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail, movement);    
    return ERROR;
}

ret_t getMovement(movementState_p *movement)
{
    if(!globalP || !movement)
        goto failXit;

    lockGlobalState();

    if(globalP->size == 0)
    {
        releaseGlobalState();
        printf("Empty list\n");
        goto failXit;          
    }
    // dequeue from head
    *movement = &g_movements[globalP->head];

    globalP->size--;
    NEXT_INDEX(globalP->head); 

    releaseGlobalState();

    return SUCCESS;
failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail, movement);    
    return ERROR;
}
