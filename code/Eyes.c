/*
	-	Bolt Closing detect to find the o-ring in the bolt tip works but is normally slow with occasional speed

*/

#include "VaporB72.h"
#include "Eyes.h"
#include "CommonFiring.h"
#include "OpenBolt.h"
#include "CloseBolt.h"
#include "Led.h"
#include "ISRs.h"
#include "Extras.h"
#include "Memory.h"
#include "Ramp.h"
#include "Trigger.h"

FILENUM(4)

//Globals
//Timer for Ball in Place count
bit bACE_TIMER_STATE = 0;
unsigned char ACE_TIMER;
bit ACE_ENABLED;				//ACE shutdown by trigger
unsigned char ACE_STATE;

//Locals
#define OPEN_BREAK_COUNT	30
#define OPEN_CONNECT_COUNT	30

#define CLOSE_GAP_COUNT		15

#define CLOSE_WAIT_COUNT	50
#define CLOSE_BREAK_COUNT	20
#define CLOSE_CONNECT_COUNT	10

void
EYE_PROCESS (void)
	{
	if (ACE_STATE && ACE_ENABLED)
		{	
		if (ACE_STATE == ACE_OPEN_BREAK)
			{  // Check if Receive Eye is being hit, ACE_RECV high means the beam is broken
			if (!ACE_RECV)
				{ // Receive Eye is being hit reset bucket
				ACE_TIMER = OPEN_BREAK_COUNT;
				}
			}
	
		if (ACE_STATE == ACE_OPEN_CONNECT)
			{  // Check if Receive Eye is not being hit
			if (ACE_RECV)
				{ // Receive Eye is not being hit
				ACE_TIMER = OPEN_CONNECT_COUNT;
				}
			}
	
		if (ACE_STATE == ACE_BREAK_ONCE)
			{
			if (ACE_RECV)
				{ //Beam is broke, go to the original process
				ACE_STATE = ACE_BEAM_BREAK + ACE_NEW_STATE;
				}
			}
		
		//Logic process for bream being broken
		if (ACE_STATE == ACE_BEAM_BREAK)
			{ // Low state on ACE_RECV means the beam is hitting the receive eye
			if (!ACE_RECV)
				{ //True (beam connect) reset ACE_TIMER
				bBallReady = 0;			//Set Global no ball in breach
				ACE_TIMER = ACE_DEBOUNCE;				  
				}
			}
		if (ACE_TIMER == 0)
			{ //Beam has been broke for 1 mSec, either bolt moved or not using extra slot on bolt
			if (ACE_STATE == ACE_OPEN_BREAK)
				{	//Done with open inhbit. Shift to next state.
				ACE_STATE = ACE_OPEN_CONNECT + ACE_NEW_STATE;  
				}	
				//Eyes have been connected for one Tic shift state, bolt should be out of the way
			if (ACE_STATE == ACE_OPEN_CONNECT)
				{
				//ACE_STATE = ACE_BEAM_BREAK + ACE_NEW_STATE;
				ACE_STATE = ACE_BREAK_ONCE + ACE_NEW_STATE;
		#if OPEN_BOLT && CLOSE_BOLT
				if (VIK_EXCAL_DIP1)
					{
		#endif
			#if OPEN_BOLT
					BEAM = BEAM_GOOD + BEAM_NEW_STATE;
					VIK_DONE_PROCESS();
			#endif
		#if OPEN_BOLT && CLOSE_BOLT
					}
				else
					{
		#endif
			#if CLOSE_BOLT
					if (EYES_MODE == EYES_DRYFIRE)	//Dryfiring, 0 out Bolt Timer, not waiting on a ball (Excalibur)
						{
						BOLT_TIMER = 1;
						}
					else
						{
						BOLT_TIMER = EYES_BOLT_OPEN;
						}
			#endif
		#if OPEN_BOLT && CLOSE_BOLT
					}
		#endif
				}	
				//Beam has stayed broke lenth of ACE_DEBOUNCE to reach here.
			if (ACE_STATE == ACE_BEAM_BREAK)
				{ 			
		#if OPEN_BOLT && CLOSE_BOLT
				if (VIK_EXCAL_DIP1)
					{//Set Global that Ball is in Breach
		#endif
		#if OPEN_BOLT
					bBallReady = 1;
					ACE_STATE = ACE_OFF + ACE_NEW_STATE;
		#endif
		#if OPEN_BOLT && CLOSE_BOLT
					}
				else
					{//Jump Excal Bolt Process to next state, pass 1 so it knows EYE_Process is making the call
		#endif
			#if CLOSE_BOLT
					BOLT_CLOSE_PROCESS(1);
			#endif
		#if OPEN_BOLT && CLOSE_BOLT
					}
		#endif
				}
			}
		if (ACE_STATE & ACE_NEW_STATE)
			{
			ACE_STATE ^= ACE_NEW_STATE;				//XOR out the new state request
			switch (ACE_STATE)
				{
				case (ACE_OFF):
					ACE_XMIT = 1;					//Shut down eye transmitter
					return;
					
				//Xmit Power on, looking for beam to break for 3 mSec
				case (ACE_OPEN_BREAK):
					ACE_XMIT = 0;
					ACE_TIMER = OPEN_BREAK_COUNT;
					break;
				
				//Xmit Power on, looking for eyes to connect for 3 mSec
				case (ACE_OPEN_CONNECT):
					ACE_XMIT = 0;
					ACE_TIMER = OPEN_CONNECT_COUNT;
					break;
			
				case (ACE_BREAK_ONCE):
					ACE_XMIT = 0;
					break;
				//Power on eye, looking for bream to break for ACE_DEBOUNCE length of time
				case (ACE_BEAM_BREAK):
					ACE_XMIT = 0;
					ACE_TIMER = ACE_DEBOUNCE;
					break;		
				default:
					break;
				}
			}
		}
	}


/*
	Called from QRTR_SEC process
*/
void
EYE_TOGGLE_CHECK (void)
  {	
  if (EYES_MODE > EYES_ALWAYS) //Trigger disable not wanted for EYES_OFF or EYES_ALWAYS
	{
	//Check for trigger down for 5 seconds
	if (!Trigger)
	  {
		if (QRTRSEC >= 20)
		  {
			if (ACE_ENABLED == 0)
			  { // Eye system was off turn it on
				ACE_STATE = ACE_OFF + ACE_NEW_STATE;
				// Update with proper value for eyes off MAX ROF
#if RAMP
				MAX_ROF_OFFSET = ROF_ARRAY[EYES_MAX_ROF];
				ShotOffset = MAX_ROF_OFFSET;
#endif
				BEAM = BEAM_GOOD + BEAM_NEW_STATE;
				ACE_ENABLED = 1;
				QRTR_RESET();
			  }
			else
			  { // Eye system was on so turn if off
				ACE_STATE = ACE_OFF;
				// Update with proper value for MAX_ROF
#if RAMP		
				MAX_ROF_OFFSET = ROF_ARRAY[BYPASS_MAX_ROF];
				ShotOffset = MAX_ROF_OFFSET;
#endif				
				BEAM = BEAM_HardOverRide + BEAM_NEW_STATE;
				ACE_ENABLED = 0;
				bBallReady = 1;		//Override ball detect for Viking Mode
				QRTR_RESET();
			  }
		  }
	  }
	}
  }  

void
ACE_TEST (void)
  {
	ACE_XMIT = 0;		//Turn on Transmit Eye
	BOLT_TIMER = 100;
	for(;;)
	  {
		if (BOLT_TIMER == 0)
		  {
		    BOLT_TIMER = 100;			
			if (ACE_RECV)
			  {
				LED = 1;
			  }
			else
			  {
				LED = 0;
			  }
		  }
#if POWERSAVE == 1
		IDLE();
#endif
		TICS = TICS_BUFFER;
	  }
  }
  