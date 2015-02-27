#include "VaporB72.h"
#include "Trigger.h"
#include "CommonFiring.h"
#include "ISRs.h"
#include "Memory.h"
#include "Ramp.h"
#include "Extras.h"

FILENUM(9)
//Globals
// How long trigger must be stable to be considered debounced
unsigned char DEBOUNCE_TIMER;
// How long must pass since trigger pull before trigger can be reactivated
unsigned char CYCLE_INHIBIT_TIMER;
//# of Tics from Previous to Current
unsigned char TriggerTics;

bit Trigger_Previous;
//Trigger bits
volatile bit TRIGGER_USER_REQUEST =			0;		// Trigger request from user, set by ISR
bit TRIGGER_BUFFER =				0;		// Buffer for trigger pull
bit TRIGGER_FIRING =				0;		// Firing in process
bit TRIGGER_COUNTED =				0;		// Current shot counted 
bit TRIGGER_INHIBIT =				0;		// Trigger request being ignored

	//Trigger Tic on previous pull
unsigned char TriggerTicPrevious;
	//Trigger Tic on current pull
unsigned char TriggerTicCurrent;

void
POLL (void)
{
	static bit Trigger_Bounce= 0;

	if (EX0 == 0)		//If interrupt is on don't need to poll for debounce
		{
		Trigger_Bounce = Trigger_Previous ^ Trigger;	//XOR Trigger(Current) with Trigger_Previous, 1 in _Bounce indicates a change in state
		if (Trigger_Bounce)
			{ // Check if pull rate > 5
#if RAMP
			if (TriggerTics < 250)
			  {
#endif
				if (TICS_BUCKET)
					{DEBOUNCE_TIMER = DEBOUNCE + 1;}
				else
					{DEBOUNCE_TIMER = DEBOUNCE;}
#if RAMP
			  }
			else
			    {
				if (TICS_BUCKET)
					{DEBOUNCE_TIMER = DEBOUNCE + SLOW_DEBOUNCE + 1;}
				else
					{DEBOUNCE_TIMER = DEBOUNCE + SLOW_DEBOUNCE + 1;}
				}
#endif
			Trigger_Previous = Trigger;	//Save new trigger state for next poll loop
			}
		if (DEBOUNCE_TIMER == 0)
			{
			if (Trigger) 
				{
				IE0 = 0;
				EX0 = 1;
				if (bDouble)
					{//Response Mode, put another shot in buffer on release
					SHOT_BUFFER += SHOTVALUE;
					}
				if ((MOD_EF == BREAKOUT) && bAuto)
					{bAuto = 0;}	// First shot done, turn off full auto
				bForceDone = 0; // Resetting to 0 in case a ForceShot had been done in Viking Mode
				return;		//Trigger is up, interrupt back on, done with function till next time
				}
			else
				{
				DEBOUNCE_TIMER = 1; //Check next tic
				if (bAuto)
					{ //Trigger is debounced down, Previous shot is done, Full Auto conditions are met
					SHOT_BUFFER = (SHOTVALUE*2);	//Put another shot in buffer
					QRTR_RESET();
					}
				}
			}
		}
}

#if RAMP
void
COUNT_PULLS (void)
  {  //Check for something to count
	if (TRIGGER_BUFFER && (!TRIGGER_COUNTED))
	  {
		TRIGGER_COUNTED = 1;			//Set flag so we know shot has been counted
		// Check if a 2nd or higher shot
		if (SemiShots)
		  {
			TriggerTicPrevious = TriggerTicCurrent;
			TriggerTicCurrent = TICS;
			//TriggerTics = ComputeOffset (&TriggerTicCurrent, &TriggerTicPrevious);
			TriggerTics = ComputeOffset (TriggerTicCurrent, TriggerTicPrevious);		
		  }
		else
		  { // First shot so just record TICS
			TriggerTicCurrent = TICS;
			TriggerTics += 255;
		  }
		// Check if we need to add in SLOW_DEBOUNCE
		if (TriggerTics > 250)
			{DEBOUNCE_TIMER = DEBOUNCE + SLOW_DEBOUNCE;}
		// Check if we are still counting up to requested by slot
		if ((SemiShots < EFS_PULLS) && TRIGGER_FIRING)  //Only count if we actually are firing
		  {
			SemiShots++;							//Increment shots
		  }
	  }
  }
#endif

#if RAMP
unsigned char  
ComputeOffset (unsigned char current_,unsigned char previous_)
  {
	unsigned char offset_;
	
	if (QRTRSEC)
		{
		offset_ = 255;
		return offset_;
		}
	if (current_ >= previous_)
	  {
		offset_ = (current_ - previous_);
	  }
	// Tics looped through 255
	else
	  {
		offset_ = (0xFF - previous_);			//Get # of Tics before rollover
		offset_ += current_;			// Add # since rollover
		offset_++;							// Add 1 because 0 is a valid Tic.
	  }
	return offset_;
  }
#endif

 /*
	Delay before allowing Trigger to become active after a shot even if debounced.
	Should prevent the forwards stroke of the hammer ram from causing the trigger to be driven forwards against a finger causing
	kick induced trigger pull to be registered
  */
#if CYCLIC
/* Moved entirely into Tic Timer
void
CYCLE_INHIBIT_PROCESS (void)
  {
	if (CYCLE_INHIBIT_TIMER)
	  {  //Time has passed, release trigger
		TRIGGER_INHIBIT = 0;
	  }
  }
*/
#endif
