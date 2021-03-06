#include "stateMachine.h"

static states_t curState;
static states_t curStPID;
static states_t curStReq;

uint16_t timerVector[TSIZE];
uint8_t g_pwmDuty = TPWM_CONST;
uint8_t g_timExec = 0;
uint8_t goLeft = 0;
uint8_t goRight = 0;

ret_t initMachine()
{
  curState = IDLE;
  curStPID = IDLE;
  curStReq = IDLE;
  initTimer();
  return SUCCESS;
}

ret_t processMachine(void)
{
  ret_t ret;
  discPack_t nwPacket;
  // Change states accordingly
  smDebug("processMachine: Before curState=%d\n", curState);
  switch(curState)
  {
    case INVALID_S: // Should never come here
        printf("processMachine: Non-initialized state machine\n");
        return ERROR;
       break; 
    case IDLE:      // Idle state, should not do nothing but expect an init
        goLeft = FALSE;
        goRight = FALSE;
        if (INIT_FLAG)
        {
          if(curMov.command == ROTATE_RIGHT)
            goRight = TRUE;
          if(curMov.command == ROTATE_LEFT)
            goLeft = TRUE;
          g_timExec = curMov.time;
          // Reset 'till TEXEC as other timer values are used for other cases
          memset(timerVector, 0, TSAVE_DATA*sizeof(uint16_t));
          curState = INIT_MOV0;    
        }
       break;
    case INIT_MOV0: // INIT states, force 100% speed for a specified time
       if (GO_LEFT)
         goLeft = TRUE;
       else if (GO_RIGHT)
         goRight = TRUE;
       if (GO_FWD)
         curState = INIT_MOV1;
       else
         curState = INIT_MOV2;
       break;
    case INIT_MOV1: // Go full FWD
       if (TIMER_START_DONE)
         curState = INIT_MOV3;
       break;
    case INIT_MOV2: // Go full BWD
       if (TIMER_START_DONE)
         curState = INIT_MOV3;
       break;
    case INIT_MOV3: // Prepare PWM stuff
        if (GO_FWD)
         curState = FRONT0;
        else
         curState = BACK0; 
       break;
    case FRONT0:
        if (DUTY_REACHED)
          curState = FRONT1;
        break;
    case FRONT1:
        if (FREQ_REACHED)
          curState = FRONT2;
        break;
    case FRONT2:  // Used to restart timer counter
        if (TIMER_EXEC_MAX)
        {
          curState = IDLE;
        }
        else
        {
          timerVector[TPWM] = 0;
          curState = FRONT0;
        }
        break;
    case BACK0:
        if (DUTY_REACHED)
          curState = BACK1;
        break;
    case BACK1:
        if (FREQ_REACHED)
          curState = BACK2;
        break;
    case BACK2:  // Used to restart timer counter
        if (TIMER_EXEC_MAX)
        {
          curState = IDLE;
        }
        else
        {
          timerVector[TPWM] = 0;
          curState = BACK0;        
        }
        break;
    default:
        break;
  }
  smDebug("processMachine: After curState=%d\n", curState);

  // Process outputs
  EXEC_N_CHECK(processOutputs(), ret);

  // Distributed PID state machine
  smDebug("processMachine: Before curStPID=%d\n", curStPID);
  switch(curStPID)
  {
    case INVALID_S: // Should never come here
        printf("processMachine: Non-initialized state machine\n");
        return ERROR;
       break; 
    case IDLE:      // Idle state,  get our position
       if (readPosition() == SUCCESS)
         curStPID = SEND_POS;
       else
        smDebug("processMachine: Error getting position\n");
       break;
    case SEND_POS:
       if (sendPosMulticast() == SUCCESS)
       {
         curStPID = WAIT_RES;
         // Start timer val
         timerVector[TRESP_TIMEOUT] = 0;
       }
       else
        smDebug("processMachine: Error multicasting position\n");
       break;
    case WAIT_RES:
       if (getResponse() == (uint8_t)MIN_RESP_CONST || TIMER_RSP_TIMEOUT)
       {
         curStPID = CALC_PID;
       }
       else
        smDebug("processMachine: Not enough responses yet\n");
       break;
    case CALC_PID:
       if (processMyPID() == SUCCESS)
         curStPID = IDLE;
       break;
    default:
       break;
  }
  smDebug("processMachine: After curStPID=%d\n", curStPID);

  // Machine to calculate requests from other robots
  smDebug("processMachine: Before curStReq=%d\n", curStReq);
  switch(curStReq)
  {
    case INVALID_S: // Should never come here
        printf("processMachine: Non-initialized state machine\n");
        return ERROR;
       break; 
    case IDLE:
       if (getRequest(&nwPacket) == SUCCESS)
         curStReq = CALC_RES;
       break;
    case CALC_RES:
       if (processResponsePID(&nwPacket) == SUCCESS)
         curStReq = IDLE;
       break;  
    default:
       break;
  }
  smDebug("processMachine: After curStReq=%d\n", curStReq);

  // Save our position each 5 seconds
  if (T_SAVE_DATA)
  {
    smDebug("processMachine: Saving actual position.\n");
    savePosition();
  }
  return SUCCESS;
}

ret_t processOutputs()
{
   FRONT_PIN = (curState == INIT_MOV1) || (curState == FRONT0) ? 1 : 0;
   BACK_PIN = (curState == INIT_MOV2) || (curState == BACK0) ? 1 : 0;
   RIGHT_PIN = goLeft;
   LEFT_PIN  = goRight; 


   return SUCCESS;
}

void initTimer()
{  
  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Clock value: 125.000 kHz
  // Mode: Normal top=0xFF
  // OC0A output: Disconnected
  // OC0B output: Disconnected
  // Timer Period: 0.104 ms
  TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
  TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (1<<CS00);
  TCNT0=0xF3;
  OCR0A=0x00;
  OCR0B=0x00;
  
  // Global disable interrupts
  #asm("cli") 
  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);  
  // Global enable interrupts
  #asm("sei") 
}

void stopTimer()
{
  // Global disable interrupts
  #asm("cli") 
  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (0<<TOIE0);  
  // Global enable interrupts
  #asm("sei") 
}

ret_t setPWMDuty(uint8_t val)
{
  if (val <= (uint8_t)TPWM_MAX)
  {
    g_pwmDuty= val;
    return SUCCESS;
  }
  printf("Error, max frequency is: %d\n", TPWM_MAX);
  return ERROR;  
}
