#include "VaporB72.h"
#include "OpenBolt.h"
#include "CommonFiring.h"
#include "ISRs.h"
#include "Led.h"
#include "Eyes.h"
#include "Trigger.h"
#include "Memory.h"

#if OPEN_BOLT
FILENUM(10)

void
VIK_BOLT_PROCESS (void)
	{
    if (TRIGGER_FIRING || (BOLT_STATE & BOLT_VIK_REQ_FLAG))
		{
		if (BOLT_TIMER == 0)
			{
			if (BOLT_BUCKET == 0)
				{ //Each flag indicates what state the Bolt_Timer is counting. Timeout will jump to next stage process
				switch (BOLT_STATE) {
					case BOLT_OFF_FLAG:
						return;
					case BOLT_VIK_REQ_FLAG:
						VIK_REQ_PROCESS();
						break;
					case BOLT_VIK_START_FLAG:
						VIK_OPEN_PROCESS();
						break;
					case BOLT_VIK_OPEN_FLAG:
						VIK_DONE_PROCESS();
						break;
					
					default:
						// should never be in this process
						break;
					}
				}
			}
		}
	}
	

 void
 VIK_OPEN_PROCESS (void)
  {
	if (ACE_ENABLED == 0)
	  {  //Eyes are disabled by trigger, use value for BYPASS_ROF because firing cycle will not complete until BOLT_TIMER is done
		BOLT_TIMER = BOLT_DELAY;
		BOLT_BUCKET = TICS_BUCKET;
	  }
	else
	  { 
		if ((BEAM & BEAM_SoftOverRide)>0)
		  { //Eyes are apparently faulting, use value for BYPASS_ROF
			BOLT_TIMER = BOLT_DELAY;
			BOLT_BUCKET = TICS_BUCKET;
		  }
		else
		  {//Eyes are active, set to huge number and allow eyes to do the work
			BOLT_TIMER = EYES_BOLT_OPEN;
			BOLT_BUCKET = TICS_BUCKET;
		  }
	  }
	//Disable eyes, dwell routine when turn on eyes when solenoid is turned off
	ACE_STATE = ACE_OFF + ACE_NEW_STATE;
	//Update state to match what Bolt_Timer is counting
	BOLT_STATE = BOLT_VIK_OPEN_FLAG;
  }

void
VIK_DONE_PROCESS (void)
  {
	if (TRIGGER_FIRING)
	  {// Allow to start another shot
		TRIGGER_FIRING = 0;
	  }
	if (!ACE_ENABLED)
	  {bBallReady = 1;}				//Override Viking Ball check to allow next shot with ACE off
	BOLT_TIMER = 0;
	BOLT_STATE = BOLT_OFF_FLAG;		//Bolt is back in open (home) position
  }

void
VIK_REQ_PROCESS (void)
  {
	if (!(ACE_STATE == ACE_BEAM_BREAK))
	  {
		if (BEAM & (BEAM_Failed_1+BEAM_Failed_2))
		  {	
			BEAM = BEAM << 1;
			BEAM = BEAM + BEAM_NEW_STATE;
		  }
		if ((BEAM & BEAM_GOOD) > 0)
		  {
			BEAM = BEAM_Failed_1 + BEAM_NEW_STATE;
		  }
		if (BEAM & BEAM_SoftOverRide)
		  { //override mode, set global flag to read that ball is in place
			bBallReady = 1;
		  }
	  }
	SHOT_BUFFER = 0;
	BOLT_STATE = BOLT_OFF_FLAG;
  }
#endif

void
FORCE_SHOT_CHECK (void)
{
 	#if PANDORA && CLOSE_BOLT
	if (VIK_EXCAL_DIP1)
		{
	#endif
		if (EYES_MODE == EYES_ALWAYS)
			{
			if (!Trigger && !bForceDone)
				{
				if (QRTRSEC >= 2)
					{ 
					bForceShot = 1;		//Setting bForceShot to override bBallReady in FireShot process
					SHOT_BUFFER = SHOTVALUE;	//Requesting a shot
					QRTR_RESET();
					}
				}
			}
	#if PANDORA && CLOSE_BOLT
		}
	#endif
}
 
 