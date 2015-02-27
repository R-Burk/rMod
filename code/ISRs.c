#include "VaporB72.h"
#include "ISRs.h"
#include "CommonFiring.h"
#include "Eyes.h"
#include "Memory.h"
#include "Trigger.h"
#include "Ramp.h"
#include "OpenBolt.h"
#include "Ramp.h"
#include "Extras.h"
#include "Led.h"

FILENUM(5)

// Global Tics counter, note this can never be reset or it would mess up every timer being used
volatile unsigned char TICS = 0;
volatile unsigned char TICS_BUCKET = 0;
volatile unsigned char TICS_BUFFER;
// Longer time tracks
unsigned char MINUTES;
unsigned char QRTRSEC;

//Timers that run based off TICS
unsigned char QRTR_TIMER = 250;
//Dwell Variables
unsigned char DWELL_HIGH;
unsigned char DWELL_LOW;
unsigned char DWELL_HIGH_ARS;
unsigned char DWELL_LOW_ARS;


//Locals
/*Default crystal speed can change later */
#define CRYSTAL 12000000

/* 12 cycles per count */
#define Calc_Ms ((1000 * 1000000)/(CRYSTAL / 12))
//unsigned int MILLISEC = (unsigned int)(Calc_Ms);
//const unsigned int TENTHMILLISEC = 100;
#define TENTHMILLISEC	100


/* Value to preload Timer 0 with so it will reach full at one millisecond */
//#define Tic_Reload_Value (65536 - MILLISEC)
//#define Tic_High Tic_Reload_Value >> 8
//#define Tic_Low Tic_Reload_Value + 17

void
QRTR_SECOND (unsigned char trigmode)
	{
    if (!QRTR_TIMER)
		{ //Quarter Second
		QRTRSEC++;
		QRTR_TIMER = 250;
		// 120 1/2 second = 1 minute
		if (QRTRSEC >= 240) 
			{ //Increment minutes and reset QRTRSEC
			MINUTES++;
			QRTRSEC = 0;
			}
		// Functions dependent on QRTRSEC and only need checked when QRTRSEC increments up
		if (trigmode == 0)
			{
			EYE_TOGGLE_CHECK();
			FORCE_SHOT_CHECK();
#if RAMP
			/*Check if ARS is enabled*/
			if (ARS_TIME)
				{/*Check if enough time has passed, 1 = 15seconds, 2 = 30 seconds, 3 = 45 seconds*/
				if (QRTRSEC >= (ARS_TIME * 60))
					{/*Add ARS to Dwell_Bucket */
					TL1 = DWELL_LOW_ARS;
					TH1 = DWELL_HIGH_ARS;
					}
				}
			//Was Check_EFS_TIME
			//Check if a second has passed since last pull
			if (QRTRSEC >= 4)
				{
				SemiShots = 0;
				ShotOffset = MAX_ROF_OFFSET;
				if (EF_MODE == AUTO)
					{ bAuto = 0; } //Let off trigger, turn off Full Auto Trigger requests
				bDouble = 0;
				ET0 = 0;	//Disable Timer 0
				TR0 = 0;	//Stop Timer 0 counting
				TL0 = RELOAD_TIC_SHORT;
				}
#endif
			}
		}
#if POWERSAVE == 1
	if (MINUTES >= 15)
		{  //15 minutes of idling
		POWER_DOWN();
		QRTR_RESET();
		}
	if (QRTRSEC || MINUTES)
		{	//Power Saving
		IDLE();
		}
#elif POWERSAVE == 2
	if (QRTRSEC || MINUTES)
		{
		SIM_IDLE();
		}
#endif	  
	}
  
/* 
	Resets HALF_SECOND to a zero baseline
*/
void
QRTR_RESET (void)
  {
	MINUTES = 0;
	QRTRSEC = 0;
	QRTR_TIMER = 250;
  }

//Calculates values for Timer Dwell
void
CALC_DWELL (void)
  { 
	unsigned int DWELL_RELOAD_VALUE;
	unsigned char i;
   
   DWELL_RELOAD_VALUE = (unsigned int)DWELL_MAJOR * 10;
   DWELL_RELOAD_VALUE = (unsigned int)TENTHMILLISEC * DWELL_RELOAD_VALUE;
	if (DWELL_MINOR)
	  {
		DWELL_RELOAD_VALUE = (unsigned int)(DWELL_RELOAD_VALUE + (TENTHMILLISEC * DWELL_MINOR));
	  }
	DWELL_RELOAD_VALUE = (unsigned int)(0xFFFF - DWELL_RELOAD_VALUE);
	DWELL_LOW = (unsigned char)(DWELL_RELOAD_VALUE);
	DWELL_HIGH = (unsigned char)(DWELL_RELOAD_VALUE >> 8);
	
	i = (unsigned char)(DWELL_MAJOR + ARS);
	DWELL_RELOAD_VALUE = (unsigned int)(TENTHMILLISEC * i * 10);
	if (DWELL_MINOR)
	  {
		DWELL_RELOAD_VALUE = (unsigned int)(DWELL_RELOAD_VALUE + (TENTHMILLISEC * DWELL_MINOR));
	  }
	DWELL_RELOAD_VALUE = (unsigned int)(0xFFFF - DWELL_RELOAD_VALUE);
	DWELL_LOW_ARS = (unsigned char)(DWELL_RELOAD_VALUE);
	DWELL_HIGH_ARS = (unsigned char)(DWELL_RELOAD_VALUE >> 8);
 }

void
TriggerISR (void) interrupt 0 //using 1
  {
	// Check if we are blocking trigger requests
	if (TRIGGER_INHIBIT) 
	  {return;}

  EX0 = 0;							//Disable Interrupt 0
	IE0 = 0;							//Clear request
	
	// Initialize Debounce timer
	DEBOUNCE_TIMER = DEBOUNCE;
//	DEBOUNCE_BUCKET = TICS_BUCKET;
	Trigger_Previous = Trigger_Closed;	//Set initial state for debouncing
	TRIGGER_USER_REQUEST = 1;		//Not using Trigger_Buffer(s) so I don't run into a problem with routing doing a decrement
  }


void
TicTimer0ISR (void) interrupt 1 //using 1 // .1 mSec Tic generator, 8 bit auto reload for precision
  {
//	TF0 = 0;									//Clear overflow flag, not needed done by hardware when it interrupts
	TICS_BUCKET++;								//Fill the tics bucket
	if (ACE_TIMER)
		{ACE_TIMER--;}				//Drain the ace bucket
	if (BOLT_TIMER == 0)
		{					
		if (BOLT_BUCKET) 
			{BOLT_BUCKET--;}		//Drain the bolt bucket if timer is empty
		}
#if POWERSAVE == 2
	bSimIdle = 0;
#endif
  }

void
Dwell1ISR (void) interrupt 3 //using 2
  {
	ET1 = 0;							//Turn off Timer
    TR1 = 0;							//Stop Counting
//	TF1 = 0;							//Clear overflow
#if OPEN_BOLT && CLOSE_BOLT
	if (VIK_EXCAL_DIP1)
	  {
#endif
  #if OPEN_BOLT
		SOLENOID2 = SOLENOID_OFF;		//Turn off Viking hammer solenoid
		ACE_STATE = ACE_OPEN_BREAK + ACE_NEW_STATE;	//Start ACE Process
  #endif
#if OPEN_BOLT && CLOSE_BOLT
	  }
	else
	  {
#endif
  #if CLOSE_BOLT
		SOLENOID1 = SOLENOID_OFF;		//Turn off Excalibur hammer Solenoid
		BOLT_TIMER = 0;					//Start timer for Bolt Process
		BOLT_BUCKET = BOLT_DELAY;
		BOLT_TIMER = 0;
		BOLT_STATE = BOLT_DELAY_FLAG;	//Let Bolt Process know it's in the Delay State
  #endif
#if OPEN_BOLT && CLOSE_BOLT
	  }
#endif
	TL1 = DWELL_LOW ;					//Reload Timer for next use
	TH1 = DWELL_HIGH;
  }

/*
 	Global TICS increment. Using Timer 2 for 16 bit auto reload function.
		Saves code and offers more precision	
*/
void
TicTimer2ISR (void) interrupt 5 //using 1	// 1 mSec Tic generator
{
	TF2 = 0;	//Clear interrupt flag
	TICS_BUFFER++;							//Move the tics along
	TICS_BUCKET = 0;  						//Empty the tics bucket
	//Drain the timers if not empty
	if (BOLT_TIMER)
		{
		BOLT_TIMER--;
		}
	if (DEBOUNCE_TIMER)
		{DEBOUNCE_TIMER--;}
		
#if CYCLIC
	if (CYCLE_INHIBIT_TIMER)
		{
		CYCLE_INHIBIT_TIMER--;
		if (CYCLE_INHIBIT_TIMER == 0)
			{  //Time has passed, release trigger
			TRIGGER_INHIBIT = 0;
			}
		}
#endif
#if RAMP
	if (SHOT_OFFSET_TIMER)
		{SHOT_OFFSET_TIMER--;}
#endif
#if POWERSAVE == 2
	bSimIdle = 0;
#endif
	if (QRTR_TIMER)
		{QRTR_TIMER--;}
}
