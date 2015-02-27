#include "VaporB72.h"
#include "CloseBolt.h"
#include "CommonFiring.h"
#include "Trigger.h"
#include "Led.h"
#include "ISRs.h"
#include "Eyes.h"
#include "Memory.h"
#include "Extras.h"
#include "Ramp.h"

#if CLOSE_BOLT
FILENUM(2)

static void
EXCAL_BOLT_PROCESS (void)
	{
    if (TRIGGER_FIRING)
		{
		if (BOLT_TIMER == 0)
			{
			if (BOLT_BUCKET == 0)
				{ //Each flag indicates what state the Bolt_Timer is counting. Timeout will jump to next stage process
				switch (BOLT_STATE) {
					case BOLT_OFF_FLAG:
						return;
					case BOLT_DELAY_FLAG:
						BOLT_OPEN_PROCESS();
						break;
					case BOLT_OPEN_FLAG:
						BOLT_CLOSE_PROCESS(0);
						break;
					case BOLT_CLOSE_FLAG:
						BOLT_DONE_PROCESS();
						break;
					default:
						// should never be in this process
						break;
					}
				}
			}
		}
	}

  void BOLT_OPEN_PROCESS (void)
  //
  {
	if ((MOD_EF == SNIPER) && bMOD_EF)
	  {	// Sniper Mode, just firing ball so jumping to end of normal firing cycle
		BOLT_DONE_PROCESS();
		return;
	  }
	// Turn on solenoid set counter to time bolt open
	SOLENOID2 = SOLENOID_ON;
	if (ACE_ENABLED == 0)
	  {  //Eyes are disabled by trigger, use value in slot
		BOLT_TIMER = BOLT_OPEN;
		BOLT_BUCKET = TICS_BUCKET;
	  }
	else
	  { 
		if ((BEAM & BEAM_SoftOverRide)>0)
		  { //Eyes are apparently faulting, use value in slot
			BOLT_TIMER = BOLT_OPEN;
			BOLT_BUCKET = TICS_BUCKET;
		  }
		else
		  {//Eyes are active, set to huge number and allow eyes to do the work
			BOLT_TIMER = EYES_BOLT_OPEN;
			BOLT_BUCKET = TICS_BUCKET;
		  }
	  }
	//Set ACE to start process for bolt opening
	ACE_STATE = ACE_OPEN_BREAK + ACE_NEW_STATE;
	//Update state to match what Bolt_Timer is counting
	BOLT_STATE = BOLT_OPEN_FLAG;
  }
  

void
BOLT_CLOSE_PROCESS (unsigned char state)
	{
	if (ACE_ENABLED)
		{
		if (state == 1)
			{ //State 1 - Eyes called into function
			BEAM = BEAM_GOOD + BEAM_NEW_STATE;	
			}
		else
			{ //State 0 - Function being called by BOLT_PROCESS
			if (ACE_STATE == ACE_BEAM_BREAK)
				{ // Eyes had detected each other when bolt moved out of way and shifted to BeamBreak so working
				BEAM = BEAM_GOOD + BEAM_NEW_STATE;
				}
			else
				{ // Eyes never saw each other apparently
				if (BEAM & (BEAM_Failed_1+BEAM_Failed_2))
					{	
					BEAM = BEAM << 1;
					BEAM = BEAM + BEAM_NEW_STATE;
					}
				if (BEAM & BEAM_GOOD)
					{
					BEAM = BEAM_Failed_1 + BEAM_NEW_STATE;
					}
				}
			}
		}
	// Turn off solenoid. Set counter to time bolt closing
	SOLENOID2 = SOLENOID_OFF;
	BOLT_TIMER = BOLT_CLOSE;
	BOLT_BUCKET = TICS_BUCKET;
	ACE_STATE = ACE_OFF + ACE_NEW_STATE;
	//Set process to move to next state once Timer or ACE allows
	BOLT_STATE = BOLT_CLOSE_FLAG;
	}
	  
void
BOLT_DONE_PROCESS (void)
  //
  {
	// Allow to start another shot
	TRIGGER_FIRING = 0;
	BOLT_STATE = BOLT_OFF_FLAG;
  }
#endif	// END CLOSE_BOLT
