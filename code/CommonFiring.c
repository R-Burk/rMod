#include "VaporB72.h"
#include "CommonFiring.h"
#include "CloseBolt.h"
#include "ISRs.h"
#include "Trigger.h"
#include "Led.h"
#include "Eyes.h"
#include "Ramp.h"
#include "Memory.h"
#include "Extras.h"

FILENUM(11)

//Globals
unsigned char BOLT_TIMER;
unsigned char BOLT_BUCKET;
	//Current MAX_OFFSET for ROF
unsigned char MAX_ROF_OFFSET;
	//Offset for EFS start
unsigned char EFS_ROF_OFFSET;

bit bBallReady =					0;		// State of ball in breach in Viking mode
bit bForceShot = 					0;		// ForceShot Request Buffer
bit bForceDone =					0;		// Note a ForceShot has been done this trigger pull

// Shot buffers
bit bShotReady		=				0;		// Next shot is ready
unsigned char SHOT_BUFFER =		0;  	// Shot queuing buffer

unsigned char BOLT_STATE = 0;

void
FIRECHECK (void)
	{
	if (TRIGGER_USER_REQUEST)				//Move Trigger request to buffer, allows only one in buffer by doing this
		{
		TRIGGER_USER_REQUEST = 0;
		if (SHOT_BUFFER < SHOTVALUE)
			{
			SHOT_BUFFER += SHOTVALUE;
			}
		else
			{
			SHOT_BUFFER = SHOTVALUE;
			}
		TRIGGER_BUFFER = 1;						//Set flag to alert shot counter we have a new shot
		TRIGGER_COUNTED = 0;
		QRTR_RESET();	  							//Restart long time counter process to 0
		}
	if (!TRIGGER_FIRING)
		{
		if (SHOT_BUFFER > (SHOTVALUE - 1))
			{	//Check if first shot or we are in a string
#if RAMP
			if (SemiShots)
				{ //Shot String, check if next shot ready
				if  (bShotReady)
					{
#endif				
					FIRE_SHOT();
#if RAMP
					}
				}
			else
				{
				FIRE_SHOT();
				}
#endif		
			}
		}
	}
 
//Ball firing process
void
FIRE_SHOT (void)
  {
#if OPEN_BOLT
  #if PANDORA && CLOSE_BOLT
	if (VIK_EXCAL_DIP1)
	  {
  #endif
		if (bForceShot || (BEAM & (BEAM_SoftOverRide + BEAM_HardOverRide)))
		  { //Forcing a shot
			bBallReady = 1;
			bForceShot = 0;
			bForceDone = 1;
		  }
		if (EYES_MODE != EYES_DRYFIRE)
		  {
		  if (!bBallReady)
			{
			if (!BOLT_TIMER && (BOLT_STATE == BOLT_OFF_FLAG))
			  {
				BOLT_TIMER = EYES_BOLT_OPEN;	//Load Bolt Timer to help track how long to wait for a ball to enter breach or determine if eyes failing
				BOLT_STATE = BOLT_VIK_REQ_FLAG;	//Set bolt state to note Viking shot requested
			  }
			 return;
		    }
		  }
		else
		  { //Viking DryFire Logic
			if (bBallReady)
			  { //Ball so do not fire but exit instead
			  SHOT_BUFFER = 0;
			  return;
			  }			  
		  }
  #if PANDORA && CLOSE_BOLT
	  }
  #endif
#endif
//Here Temporarily while trying to work on eyes controlling CLOSE_BOLT

	TRIGGER_FIRING = 1;						//Set flag to alert other processes we are firing
	ET0 = 1;								//Enable Timer 0
	TR0 = 1;								//Start Tic Bucket timer counting

	SHOT_BUFFER -= SHOTVALUE;	 			//Move requests down the buffers

#if RAMP
	bShotReady = 0;
	if (TICS_BUCKET)						//Set Shot Timer
		{SHOT_OFFSET_TIMER = ShotOffset + 1;}
	else
		{SHOT_OFFSET_TIMER = ShotOffset;}
#endif

#if CYCLIC
	TRIGGER_INHIBIT = 1;					//Set to block from allowing anymore Trigger Requests
	CYCLE_INHIBIT_TIMER = DWELL_MAJOR + CYCLE_INHIBIT;	
#endif

#if OPEN_BOLT && CLOSE_BOLT
	if (VIK_EXCAL_DIP1)
	  { //Viking - Can only use one solenoid
#endif
#if OPEN_BOLT
		TR1 = 1;							//Turn on Hammer ISR
		ET1 = 1;
		SOLENOID2 = SOLENOID_ON;
		ACE_STATE = ACE_OFF + ACE_NEW_STATE;  //Turn off ACE
		BOLT_STATE = BOLT_VIK_START_FLAG;   //Let Bolt Process know it needs to start tracking cycle time and chance eye states
		BOLT_TIMER = 0;						//Zero out timers so VIK_BOLT_PROCESS will process flag
		BOLT_BUCKET = 0;
		bBallReady = 0;						//We are firing this ball away so it no longer counts
#endif
#if OPEN_BOLT && CLOSE_BOLT
	  }
	else
	  { //Excal
#endif		  
#if CLOSE_BOLT
		if (EYES_MODE != EYES_DRYFIRE)			//Do not fire if cycling
			{
			if (MOD_EF < SNIPER)
				{
				TR1 = 1;							//Turn on Hammer ISR
				ET1 = 1;
				SOLENOID1 = SOLENOID_ON;						//Turn on Hammer Solenoid
				}
			else
				{ //Sniper Mode
				if (bMOD_EF)
					{
					bMOD_EF = 0;		//Indicate need to fire ball next trigger pull
					BOLT_OPEN_PROCESS();
					}
				else
					{
					if (!bEfsState)		//Check if we are firing slow enough to continue sniper mode
						{ // We are below EFS PULLS and ROF slots so indicate sniper process
						bMOD_EF = 1;	//Indicate need to load ball next trigger pull
						}
					TR1 = 1;					//Turn on Hammer ISR
					ET1 = 1;
					SOLENOID1 = SOLENOID_ON;
					}
				}
			}
		else
			{
			TR1 = 1;		//Turn on Hammer ISR to simulate firing
			ET1 = 1;
			}
#endif			  
#if OPEN_BOLT && CLOSE_BOLT
	  }
#endif
  }
