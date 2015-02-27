#include "VaporB72.h"
#include "Led.h"
#include "CommonFiring.h"
#include "ISRs.h"
#include "Trigger.h"

FILENUM(6)
//Globals
// Basically does PWM for LED
unsigned char LED_TIMER;
unsigned char LIGHT;
unsigned char BEAM;

//Locals
unsigned char LED_ON_TIME;
unsigned char LED_OFF_TIME;
//Aiming for a 10% duty cycle on most of these timings

//SLOW - 4 blinks a second (1000 / (25 + 225)) - About 4 hz
#define LED_SLOW_ON_TIME 	(unsigned char)25
#define LED_SLOW_OFF_TIME	(unsigned char)225

//FAST - 10 blinks a second (1000 / (12 + 54)) - About 15 Hz
#define LED_FAST_ON_TIME 	(unsigned char)12
#define LED_FAST_OFF_TIME 	(unsigned char)54

//Continuous Dim - ( 1000 / (1 + 9)) - About 50 hz, 10% duty cycle
#define LED_DIM_ON_TIME		(unsigned char)3
#define LED_DIM_OFF_TIME	(unsigned char)17

//Continuous Bright - (1000 / (8 + 2)) - About 100 hz, 80% duty cycle
#define LED_BRIGHT_ON_TIME 	(unsigned char)7
#define LED_BRIGHT_OFF_TIME 	(unsigned char)3

bit bBallPrevious = 1;		//Previous state of ball in Viking Mode

void LED_CONTROL (void)
  {
    if (LIGHT & LIGHT_NEW_STATE)						//Check state of global flag ( using I/O logic for states)
	  {
		LIGHT ^= LIGHT_NEW_STATE;			//XOR out the light change request
		switch (LIGHT)
		  {
			case (LIGHT_OFF):						//Turn off LED entirely
				LED = 1;
				LED_ON_TIME =	0;
				LED_OFF_TIME =	255;
				break;

			case (LIGHT_SLOW):						//Blink Slow
				LED_ON_TIME = LED_SLOW_ON_TIME;
				LED_OFF_TIME = LED_SLOW_OFF_TIME;
				break;
				
			case (LIGHT_FAST):						//Blink Fast
				LED_ON_TIME = LED_FAST_ON_TIME;
				LED_OFF_TIME = LED_FAST_OFF_TIME;
				break;
			
			case (LIGHT_SOLID_DIM):				//Solid Dim
				LED_ON_TIME = LED_DIM_ON_TIME;
				LED_OFF_TIME = LED_DIM_OFF_TIME;
				break;
				
			case (LIGHT_SOLID_BRIGHT):				//Solid Bright
				LED_ON_TIME = LED_BRIGHT_ON_TIME;
				LED_OFF_TIME = LED_BRIGHT_OFF_TIME;
				break;
			
			case (LIGHT_BLINK_ONCE):
				LED_ON_TIME = LED_BRIGHT_ON_TIME;
				LED_OFF_TIME = LED_BRIGHT_OFF_TIME;
				BOLT_TIMER = 50;
				break;
				
			case (LIGHT_FAST_ONCE):
				LED_ON_TIME = LED_FAST_ON_TIME;
				LED_OFF_TIME = LED_FAST_OFF_TIME;
				BOLT_TIMER = 100;
				break;
				
			default:						//Shouldn't get to here
				LIGHT = LIGHT_OFF + LIGHT_NEW_STATE;
				break;
		  }
		LED_TIMER = TICS + LED_ON_TIME;		//Set LED_TIMER to fire on next On Time Tic
	  }
	if (TICS == LED_TIMER)                 //Check if Time to flip state of LED
	  {
		if (LED && LIGHT)					// I/O pin check (inverted logic)
		  {			
			LED = 0;						//Turn on LED
			LED_TIMER = TICS + LED_ON_TIME;	//Set Timer to fire after desired on time for LED
		  }
		else
		  {
			LED = 1;						//Turn off LED (Vcc on Pin)
			LED_TIMER = TICS + LED_OFF_TIME;//Set Timer to fire after desired off time
		  }
		// Catch _Once and change to stay off. Can use Bolt_Timer because only doing this in Program Mode
		if ((LIGHT == LIGHT_BLINK_ONCE) || (LIGHT == LIGHT_FAST_ONCE))
		  {
			if (BOLT_TIMER == 0)
			  {
				LIGHT = LIGHT_OFF + LIGHT_NEW_STATE;
			  }
		  }
	  }
  }

/***********************************************************************
 *                    
 *  FUNCTION     :  LED_READ_OUT()
 *  DESCRIPTION  :  Produces one blink cycle per value requested. This process violates
					cyclic executive and does not return till done.
 *  PARAMETERS   :  unsigned char* - address of variable for # of blinks being requested
 *  RETURNS      :  N/A
 *
 ***********************************************************************/ 
void
LED_READ_OUT(unsigned char Blinks)
  {
	LIGHT = LIGHT_OFF + LIGHT_NEW_STATE;
	QRTR_RESET();
	while (Blinks)
	  { 
		QRTR_SECOND(0);
		// Check if off and 1/2 second
		if (LED && (QRTRSEC == 2))
		  { //Turn on , reset Qrtr
			LED = 0;
			QRTR_RESET();
		  }
		 // Check if LED on and 1/2 second
		if ((!LED) && (QRTRSEC ==2))
		  { //Turn off LED
			LED = 1;
			QRTR_RESET();
			Blinks--;
		  }
		TICS = TICS_BUFFER;
	  }
	// No blinks left, setup and move to next state
	TRIGGER_USER_REQUEST = 0;
	SHOT_BUFFER = 0;
	QRTR_RESET();
  }

void LedByBeamState (void)
{
	if (BEAM & BEAM_NEW_STATE)
	  {
		BEAM ^= BEAM_NEW_STATE;		//XOR out the New State
		switch (BEAM)
		  {
			case (BEAM_GOOD):
			  LIGHT = LIGHT_SOLID_BRIGHT + LIGHT_NEW_STATE;
			  break;
			case (BEAM_Failed_1):	//Fall throught logic, need to check the state but want both to blink the same
			case (BEAM_Failed_2):
			  break;
			case (BEAM_SoftOverRide):
			  LIGHT = LIGHT_SOLID_DIM + LIGHT_NEW_STATE;
			  break;
			case (BEAM_HardOverRide):
			  LIGHT = LIGHT_FAST + LIGHT_NEW_STATE;
			  break;
			default:
				//Shouldn't end up here, turning off LED to indicate a fault in logic somewhere
			  LIGHT = LIGHT_OFF + LIGHT_NEW_STATE;
			  break;
		  }
	  }
#if OPEN_BOLT && CLOSE_BOLT
	if (VIK_EXCAL_DIP1 && (BEAM & BEAM_GOOD))
#else
	if (BEAM & BEAM_GOOD)
#endif
	  {
		if (bBallPrevious == !bBallReady)  //Check if ball state has changed
		  {
			bBallPrevious = bBallReady;	//Save state for later compare
			if (bBallReady)
			  {LIGHT = LIGHT_SOLID_BRIGHT + LIGHT_NEW_STATE;}
			else
			  {LIGHT = LIGHT_SLOW + LIGHT_NEW_STATE;}
		  }
	  }
}


  