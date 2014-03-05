
#ifndef __AI__
#define __AI__

#include "../platform.h"
#include "../movement/pid.h"      // To modify left and right movement times
#include "../network/network.h"   // Send and recieve messages
#include "../nrf24l01/nrf24l01.h" // To check for valid pipes
#include <stdint.h>

#define MAX_HIST_POS 5
#define MAX_NO_RESPONSES 5

/* The global position and angle */
typedef struct {
  uint8_t x;
  uint8_t y;
  int16_t angle; 
}
pos_t;
typedef pos_t* pos_p;

/* Global variables */
extern pos_t g_pos; 
extern pos_t historicalPos[MAX_HIST_POS]; // Used to save historical positions
extern int16_t robotResp[MAX_NO_RESPONSES]; // Save responses of all robots 
extern uint8_t isValidPos;                  // Flag to save a valid position

/* 
 * Function to obtain the PID required value using multicast to all nodes in
 * the network. 
 */
extern ret_t processMyPID(void);

/* 
 * Function to process and send the response to any other robot.
 */
extern ret_t pronessResponsePID(void);

/* 
 * Function to multicast the current position of this robot.
 */
extern ret_t sendPosMulticast(void);

/* 
 * Function to get, if there is any, an incomming message from the network.
 * A response from all robots connected.
 *
 * @returns char number of responses currently got.
 */
extern uint8_t getResponse(void);

/* 
 * Function to get, if there is any, an incomming request from any member in
 * the network.
 *
 * @returns SUCCESS if a valid request was got.
 * @returns WARNING if a no valid request was got.
 */
extern ret_t getRequest(void);

/* 
 * Function to read the real position of the robot. 
 * 
 * @returns SUCCESS if a valid position was got.
 * @returns WARNING if a no valid position was got.
 */
extern ret_t readPosition(void);

/* 
 * Function to save current position on memory 
 */
extern ret_t savePosition(void);

#endif /* __AI__ */
