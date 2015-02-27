#include "VaporB72.h"
#include "Extras.h"
#include "Memory.h"
#include "Trigger.h"
#include "ISRs.h"
#include "Led.h"
#include "Eyes.h"
#include "CloseBolt.h"

/*
	This file contains all the extra little things not needed for normal semi operation or for enhanced mode opertions
*/

/*
	Reloads Dwell Timer with the computed values for the extra dwell if ARS_TIME has been met
*/
FILENUM(3)

#if POWERSAVE == 1
void
IDLE (void)
 {
	NOP;
	PCON |= IDL;		// Chip Idle
	NOP;
 }

void
POWER_DOWN (void)
 {
	IT0 = 0;		//Set External Interrupt 0 (Trigger) to Low Level Triggering, only way to wake up
	NOP;
	NOP;
	PCON |= PD;		//Chip Power down
	NOP;
	NOP;
	IT0 = 1;		//Waking up, set Trigger back to normal Falling Edge Triggering
 }
#elif POWERSAVE == 2
bit bSimIdle = 0;
void
SIM_IDLE (void)
{
	bSimIdle = 1;
	SIM_LOOP:
	if (bSimIdle)
		{goto SIM_LOOP;}
}
#endif