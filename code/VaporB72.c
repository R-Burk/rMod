/*
	Beta 7.2 of Pandora Combo Code
	ToDo - Tourney Lock
		-	Create process to allow eyes to detect bolt closing in Excalibur mode
		-	Create Pulse Method for Excal when bolt is open
		-	Detect Ball Pinch and release
		-	True intellisignal feed to Halo
	
	Changes:
	10	-	Created new slot EYES_MODE, renamed old slot as EF_MODE
		  -	Created 5 states for EYES_MODE
	12  - Ramp stuff has to always compile in, too unified to not
		  -	Added a new slot. MOD EF and a sniper and breakout mode
      -	Corrected the DIP switches. Using DIP1 to select Viking or Excalibur operation
      -	Changed Defaults. Using Mac settings, and slots are for  Viking or Excal.
      -	Changed Bolt Delay. It now starts when bolt solenoid turns off.
      -	Tweaked trigger programming. Added check for bDouble. Made it so bDouble and bAuto are forced off every cycle.
	13	-	Renamed specific function with Excal prefix
      -	Added check to Initialize to use BYPASS_ROF_MAX if eyes are disabled
	14	-	Added bHammerSolenoid to make Dwell Decrement when in Viking or Excalibur mode
      -	Viking - Solenoid off turns eyes back on
      -	Created SineWave LED state
      -	Modded LedByBeamState to use SineWave for bBallReady when BEAM_GOOD
	15	-	Fixed eye logic for bBallReady state to turn off when fired, so it has to look for a ball
      -	worked on the SineWave LED, barely noticeable
      -	Fixed that ACE Debounce was getting set to 0 in Viking mode ( whick broke eye logic)
      -	Added logic to decrement AcE Debounce when Timer 0 is off ( which caused eye logic to never admin a ball was there)
	16	-	Switched to DwellISR logic , removed all references to Dwell_Bucket and bHammerSolenoid. More precise and faster main code.
      -	ForceShot logic for Eye Mode 2 and DryFire logic in Viking Mode added
      -	Fixed so Version now shows in Vlink. Rearranged so stats can be read in Vlink open bolt options.
	17	-	Fixed counting shots to only those that fire a shot
      -	Created defines for VLINK memory locations, changed appropriate areas to use the defines
      -	New define PRODUCTION to eliminate asserts in final output
      -	Cleaned out a bunch of redundant returns
      -	Implemented some defines to allow creating output for Chaos boards
	18	-	Broke up code in separate C files to allow greater ease of debug
	19	-	Fixed a math error in Compute_Buckets.
      -	Reduced amount of code in Tenth Tic Timer. Using TICS_BUCKET to simply add an extra mSec to timers
      -	Moved LED to TIC Timer. Reduced number of times it's listed in various modules. Only needs to check each tic
      -	Changed main code for Open and Closed bolt. Moved processes that only needed checked once to only check when TIC_BUFFER updates TICs
      -	Went through code removing unused variables.
      -	Increased speed of main loop from 200-300 uSec down to 90-120 uSec.
      -	Rewrote SHOT_BUFFER logic. It's a full byte now. Using SHOTVALUE to indicate number of shots or partial shots in buffer
        allowed smoothing and guarantees the 50%, 150% and Curved ramps work correctly.
      -	Created SIM_IDLE to allow testing for power saving in Simulator.
	20	-	Testing Closed Bolt detection using eyes
      -	Gave up on CURVED_RAMP and deleted it.
	7.1	- Closed bolt detection working using the groove cut in white bolt, going to try to use ball to bolt gap as a faster detection
      - Removed close_bolt_original value, not needed anymore
      -	Changed ACE_TIMER back to using 1/10 Tics, need more accuracy for the bolt closing detect
      -	Fixed Dryfire in Excal mode. Dwell ISR wasn't getting turned on
      -	Turned off USING commands in ISR's per developer recommendation. Saved 28 bytes of code and 17 bytes of Ram
      -	Optimized several functions to use pointers. Added pragma callee_saves to EERPOM functions. Big memory savings.
      -	Fixed bBallReady not coming on for first shot of Viking (Timer 0 was not on at boot).
      -	Added _sdcc_external_startup and put solenoid turn offs in the function. Solenoids now turn off in first 10 uSec.
	7.2 -	Fixed Force Shot causing a 2 BPS full auto in Excalibur
      -	Fixed Dry Fire in Excalibur mode
      -	Fixed eyes not reading ball after 1 second ( anotherwords had to force fire every pod reload)
*/

#include "VaporB72.h"
#include "ISRs.h"
#include "Trigger.h"
#include "Led.h"
#include "Memory.h"
#include "CloseBolt.h"
#include "OpenBolt.h"
#include "Eyes.h"
#include "Ramp.h"
#include "Extras.h"
#include "CommonFiring.h"

FILENUM(1)

void VikFIRINGLOOP	(void);
void ExcalFIRINGLOOP	(void);
void INITIALIZE 	(void);

unsigned char _sdcc_external_startup ()	//Compiler actually does this special function before anything else, modified for my use
{	//Added solenoids to they are are the very start of the program now
#if PANDORA
	SOLENOID1 = SOLENOID_OFF;		//Turn off solenoids (it's not inverted TTL logic on this one)
#endif
	SOLENOID2 = SOLENOID_OFF;
    return 0;			//Must return 0 or rest of startup doesn't occur
}

void
main ()/*Startup Logic*/
  {
//	SP = SP - 2;		//Not needed using --MAIN-RETURN option during compile now
	LOAD_PARAMS();
	INITIALIZE();		//Call routine to load variables and set initial I/O states
#if PROGRAMMING
    if (Trigger) 		//If trigger is up goto to normal
	  {
#endif
		if (EYES_MODE == EYES_TEST)
		  {
			ACE_TEST();
		  }
		else
		  {
#if OPEN_BOLT && CLOSE_BOLT
			if (VIK_EXCAL_DIP1)
			  {
#endif
  #if OPEN_BOLT
				VikFIRINGLOOP();
  #endif
#if OPEN_BOLT && CLOSE_BOLT
			  }
			else
			  {
#endif  
  #if CLOSE_BOLT
				ExcalFIRINGLOOP();
  #endif
#if OPEN_BOLT && CLOSE_BOLT
			  }
#endif
		  }
#if PROGRAMMING
	  }
	else 		/*Trigger is down at boot then we are going to set options*/
	  {
		PROGRAM_MODE();
	  }
#endif
	// Program should never reach this point	
  }

#if OPEN_BOLT
static void
VikFIRINGLOOP ()
  { 
	for(;;)
		{
		FIRECHECK();		// Power up Solenoid 1, enabled DwellTimer, set Fire State
		VIK_BOLT_PROCESS();		// Tracks bolt state and Force Shot logic
		EYE_PROCESS();		// Controls Eye Transmit power, Eye Receive state, Ball debounce
		POLL(); 	       	// Debounce trigger switch, also performs polling for release
		if (TICS != TICS_BUFFER)
			{// Anything that only needs to execute once a TIC
			ASSERT(!((TICS + 2) == TICS_BUFFER));
			TICS = TICS_BUFFER;
			LedByBeamState();		
			LED_CONTROL();		// Controls Led States and Blink rates			
	#if RAMP		
			COUNT_PULLS();		// Processes shots pulled and TriggerROF		
			EF_PROCESS();		// Enhanced fire checks
	#endif
			QRTR_SECOND(0);		// Everything that is happening on 1/4 second times or longer
			if (!bBallReady)	// Restart counters if ball isn't loaded, needed to keep eye logic working
				{
				ET0 = 1;
				TR0 = 1;
				}
			}
		}
	//Should never reach this point
  }
#endif

#if CLOSE_BOLT
static void
ExcalFIRINGLOOP (void)
	{ 
	for(;;)
		{
		FIRECHECK();		// Power up Solenoid 1, enabled DwellTimer, set Fire State
		EXCAL_BOLT_PROCESS();		// Controls Solenoid2 and by extension the bolt
		EYE_PROCESS();		// Controls Eye Transmit power, Eye Receive state, Ball debounce
		POLL(); 	       	// Debounce trigger switch, also performs polling for release
		if (TICS != TICS_BUFFER)
			{ // Processes that only have to happen once a TIC
			ASSERT(!((TICS + 2) == TICS_BUFFER));
			TICS = TICS_BUFFER;
			LedByBeamState();		
			LED_CONTROL();		// Controls Led States and Blink rates
	#if RAMP		
			COUNT_PULLS();		// Processes shots pulled and TriggerROF
			EF_PROCESS();		// Enhanced fire checks
	#endif		
			QRTR_SECOND(0);		// Everything that is happening on 1/4 second times or longer
			}
		}
	//Should never reach this point
	}
#endif

static void
INITIALIZE (void)
  {
	ASSERT(DWELL_MAJOR < 25);
	CALC_DWELL();
	TL1 = DWELL_LOW_ARS;		//Preload Timer 1 (dwell)
	TH1 = DWELL_HIGH_ARS;
	TR1 = 0;					//Make sure Timer 1 is off
	TMOD |= M0_1;				//Set Timer 1 to 16bit mode
	PT1 = 1;					//Make Timer 1 high priority
	ET1 = 0;					//Disable Timer 1 interrupt
	
#if OPEN_BOLT && CLOSE_BOLT
	if (VIK_EXCAL_DIP1)
	  {
#endif
  #if OPEN_BOLT
		BOLT_DELAY = ROF_ARRAY[BYPASS_MAX_ROF];			//Viking mode, Bolt Delay carries value for length of cycle time
		if (EYES_MODE == EYES_DRYFIRE)
		  {
			ACE_DEBOUNCE = 1;			//Overriding ACE_DEBOUNCE to a set amount in Viking Dryfire Mode.
		  }
  #endif
#if OPEN_BOLT && CLOSE_BOLT
	  }
	else
	  {
#endif
  #if CLOSE_BOLT
		BOLT_DELAY = BOLT_DELAY * 10;					//Excal mode, Bolt Delay carries value in 1/10 milliseconds for offset between solenoids
  #endif
#if OPEN_BOLT && CLOSE_BOLT
	  }
#endif
		
	TL0 = RELOAD_TIC_SHORT;			//Preload Timer 0 Low
	TH0 = RELOAD_TIC_SHORT;			//Put reload in Timer 0 High
	TMOD |= T0_M1;					//Set Timer 0 to 8 bit auto-reload
//	PT0 = 1;						//Make Timer 0 high priority
	ET0 = 1;						//Enable Timer 0
	TR0 = 1;						//Set Timer 0 Counting

	TL2 = RELOAD_TIC_LOW_LONG;		//Load Timer 2 (tics)
	TH2 = RELOAD_TIC_HIGH_LONG;
	RCAP2L = RELOAD_TIC_LOW_LONG;	//Set AutoReload for Timer2
	RCAP2H = RELOAD_TIC_HIGH_LONG;
	ET2 = 1;						//Enable Timer 2 interrupt
	TR2 = 1;						//Set Timer 2 to counting
	
	IT0 = 1;						//Set for falling edge triggering on Trigger Interrupt
	IE0 = 0;                        //Clear pending trigger requests */
	EX0 = 1;                        //Turn on Trigger ISR
	EA = 1;							//Enable all interrupts
	
	if (EYES_MODE== EYES_OFF)
	  {ACE_ENABLED = 0;}
	else
	  {ACE_ENABLED = 1;}					//Turns on Eye_Logic
	  
#if PANDORA && OPEN_BOLT && CLOSE_BOLT
	if (VIK_EXCAL_DIP1 && ACE_ENABLED)
#elif CHAOS || (PANDORA && OPEN_BOLT)
	if (ACE_ENABLED)
#else
	if (!ACE_ENABLED)
#endif
	  {
		ACE_STATE = ACE_BEAM_BREAK + ACE_NEW_STATE;	//Turn on ACE for Viking
	  }
	else
	  {
		ACE_STATE = ACE_OFF + ACE_NEW_STATE;	//Make sure ACE is off for Excal and new request 
	  }
	ACE_RECV = 1;
	//ACE_DEBOUNCE = ACE_DEBOUNCE * 10;
	ACE_DEBOUNCE = ACE_DEBOUNCE * 5;				//Making each 1 trigger pull .5 mSec verse 1 mSec previously

	BEAM = BEAM_GOOD + BEAM_NEW_STATE;				//Assume eyes working at boot up
#if CYCLIC
	if (CYCLE_INHIBIT)
	  {
		CYCLE_INHIBIT = (CYCLE_INHIBIT << 1);		//Multiply setting by 2
	  }
	else
	  {
		CYCLE_INHIBIT = 1;		//Need some non-zero value, this way it will be up as soon or sooner than debounce so nonvisible
	  }
#endif
#if RAMP	
	// set pointer to the function associate with enhanced fire mode chosen, if mode isn't Semi (0)
	if (EF_MODE > 0)
	  {
		if (EF_MODE <= EF_MODE_MAX) {
		  switch(EF_MODE) {
			case(SEMI):
			case(RESPONSE):
				break;
			case(SLOWRAMP):
			case(MEDRAMP):
			case(FASTRAMP):
			case(HARDRAMP):
				pMode = MODE_ARRAY[(EF_MODE-2)];
				break;
			case(AUTO):
				bAuto = 1;
				break;
			}
		  }
	  }
	if (MOD_EF == PSP_NXL) //Ensure PSP/NXL limits mets
		{
		if (EFS_PULLS < 3) {EFS_PULLS = 3;}
		if (EYES_MAX_ROF > 15) {EYES_MAX_ROF = 15;}
		if (BYPASS_MAX_ROF > 15) {BYPASS_MAX_ROF = 15;}		
		}
	EFS_ROF_OFFSET = ROF_ARRAY[EFS_ROF];			//TriggerTics will be compared against this for ROF checking
	if (ACE_ENABLED)
	  {
		MAX_ROF_OFFSET = ROF_ARRAY[EYES_MAX_ROF];			//Load up MAX_ROF_OFFSET, assuming Eyes will be default
		bBallReady = 0;										//Set Ball ready indicator to false for Viking mode
	  }
	else
	  {
		MAX_ROF_OFFSET = ROF_ARRAY[BYPASS_MAX_ROF];			//Set MAX_ROF_OFFSET for eyes off
		bBallReady = 1;										//Set Ball Ready indicator to true for Viking Mode
	  }
	ShotOffset = MAX_ROF_OFFSET;
	if (MOD_EF == BREAKOUT) 
	  {bAuto = 1;}										//Setting Full Auto for first pull
#endif
  }
