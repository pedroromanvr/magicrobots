
#include "movement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ret_t (*movementFuncArr[MOVE_MAX])(void *args) = {
    unusedFunc,  
    frontFunc,
    backFunc,                        
    rightFunc,
    leftFunc,
    rotateRightFunc,
    rotateLeftFunc,
    rotateRightInvertedFunc,
    rotateLeftInvertedFunc
};
globalState_t global = {NULL, NULL,0, 0};
globalState_p globalP = &global;
curMovement_t curMovement = {.curVal = 0, .isCompleted = 0, .curState = NULL};
//Array defining the correct bit combinations for each case:
unsigned char movementVals[MOVE_MAX] = {
    // Unused
    0x00,
    // front
    0x01,
    // back
    0x02,
    // right
    0x04,
    // left
    0x08,
    // rotateRight
    0x05,
    // rotateLeft
    0x09,
    // rotateRightInverted
    0x06,  
    // rotateLeftInverted
    0x0A
};

ret_t unusedFunc(void *args)
{
    /* We should never ever reach here */
    printf("%s:%d:args=%x\n", 
        __FILE__, __LINE__, args);
    return ERROR;
}
ret_t frontFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != FRONT)
        goto failXit; 
    
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}
ret_t backFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != BACK)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}
ret_t leftFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != LEFT)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}

ret_t rightFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != RIGHT)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}

ret_t rotateRightFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != ROTATE_RIGHT)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}
ret_t rotateLeftFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != ROTATE_LEFT)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}

ret_t rotateRightInvertedFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != ROTATE_RIGHT_INVERTED)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}
ret_t rotateLeftInvertedFunc(void *args)
{
    movementState_p currentP = (movementState_p)args;
    if(!currentP || currentP->command != ROTATE_LEFT_INVERTED)
        goto failXit;
        
    if(curMovement.isCompleted)
    {
        curMovement.curVal =  movementVals[currentP->command];
        curMovement.isCompleted = 0; 
        return SUCCESS;
    }
        
failXit:
    printf("%s:%d:command=%d:curMovement=%d\n", 
        __FILE__, __LINE__, currentP->command,curMovement.curVal);
    return ERROR;
}


ret_t movementCommandExecute(moveCmd_t command)
{
    movementState_p current;
    int retCode = -1;
    if(!globalP)
        goto failXit;
    current = (movementState_p)malloc(sizeof(movementState_t));    
    if(!current)
        goto failXit;
    memset(current, 0, sizeof(movementState_t));
    current->command = command;
    current->state = PENDING;
    
    while((retCode = addMovement(current)) == WARNING);
    if(retCode != SUCCESS)
        goto failXit;
/*
    retCode = (*movementFuncArr[command])(&current);
    if(retCode != SUCCESS)
        goto failXit;
*/

    return SUCCESS;    
    failXit:
    printf("%s:%d:command=%d\nretCode=%d\n", 
        __FILE__, __LINE__, command, retCode);
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

    purgeMovementList();
        
    while((retCode = queryMovement(&current)) == WARNING);
    if(retCode != SUCCESS)
        goto failXit;
        
    if(current)
    {
        retCode = (*movementFuncArr[current->command])(current);    
        if(retCode != SUCCESS)
            goto failXit;
    }   
    
    curMovement.curState = current; 

    return SUCCESS;    
failXit:
    printf("%s:%d:current=%x\nretCode%d\n", 
        __FILE__, __LINE__, current, retCode);    
    return ERROR;
}

ret_t addMovement(movementState_p movement)
{
    if(!globalP || !movement)
        goto failXit;

    lockGlobalState();
    
    if(globalP->head == NULL && globalP->tail != NULL)
    {
        releaseGlobalState();
        goto failXit;          
    } 
    //Max size of list validation    
    if(globalP->requestPending >= (char)MAX_LIST_SIZE)  
    {   
      //  printf("Error, max size reached %s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n", 
      printf("Error, max size reached %s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n",
                __FILE__, __LINE__, globalP, globalP->head, 
                globalP->tail, movement);
         releaseGlobalState();
         return FAIL;
    }
        
    if(globalP->head == NULL)
    {
        globalP->head = movement;
        globalP->tail = movement;
    }
    else
    {
        if(globalP->tail == NULL)
            goto failXit;
        globalP->tail->next = (void *)movement;
        globalP->tail = movement;
    } 
    globalP->requestPending++;
    
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

    if(globalP->head == NULL && globalP->tail != NULL)
    {
        releaseGlobalState();
        goto failXit;          
    }
    
    *movement = globalP->head;
   /* if(globalP->head != NULL)
        globalP->head = (movementState_p)globalP->head->next;  */
    
    releaseGlobalState();

    return SUCCESS;
failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%x\nmovement=%x\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail, movement);    
    return ERROR;
}
/* Garbage collecting service 
 * Clean all DONE states using single-depth mark and sweep algorithm
 */
ret_t purgeMovementList()
{
    movementState_p traveler = NULL;   
    movementState_p prev = NULL;       
    char needToFree;
    if(!globalP)
        goto failXit;
                        
    lockGlobalState();    
    
    if(globalP->head == NULL && globalP->tail != NULL)        
    {        
        releaseGlobalState();
        goto failXit;                            
    } 
    
    if(globalP->head == NULL && globalP->tail == NULL)
        return SUCCESS; //Nothing to purge on an empty list

    prev = NULL;    
    traveler = globalP->head;
    do
    {
        needToFree = 0;                              
        if(traveler->state == DONE)
        {
            needToFree = 1;
            if(prev)
            {
                prev->next = traveler->next;      
            }
            else 
            {
                globalP->head = traveler->next;
            }      
            if(!globalP->head) //if head == NULL, thus last element deleted
                globalP->tail = NULL;
        }
        prev = traveler;
        traveler = (movementState_p)traveler->next;
        if(needToFree)
        { 
            global.requestPending--;
            free(prev);
        }
    } while(traveler != globalP->tail && traveler != NULL);
    
    releaseGlobalState();
    return SUCCESS;
failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%x\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail);    
    return ERROR;
}

ret_t validateMovement()
{
    // Validate front and back pins
    char val1 = curMovement.curVal & 0x03;  
    char val2 = curMovement.curVal & 0x0C;  
    if(val1 == (char)0x03 ||
       val2 == (char)0x0C) //Both front and back or left and right are active
        goto failXit;
    
    return SUCCESS;    
        
failXit:
    printf("%s:%d:globalP=%x\nhead=%x\ntail=%xcurMovement=%d\n", 
        __FILE__, __LINE__, globalP, globalP->head, 
        globalP->tail,curMovement.curVal);    
    return ERROR;        
}