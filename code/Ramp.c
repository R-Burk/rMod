#include "VaporB72.h"
#include "Ramp.h"
#include "CommonFiring.h"
#include "Ramp.h"
#include "Trigger.h"
#include "Memory.h"
#include "ISRs.h"

FILENUM(8)


//Globals
bit bEfsState = 0;
bit bDouble = 0;
bit bAuto = 0;
bit bMOD_EF;

//Timer to control shot ROF
unsigned char SHOT_OFFSET_TIMER;
//Global Ramp Variables
	//Semi shots made with less than 1 second between shots
unsigned char SemiShots;
	//Amount of time to be used for SHOT_OFFSET_TIMER;
unsigned char ShotOffset;


code const unsigned char ROF_ARRAY[22] = {255,255,255,255,	/*4 Bps*/	250,
	/*5 Bps*/	200,	/*6 Bps*/ 	167,	/*7 Bps*/	143,	/*8 Bps*/	125,
	/*9 Bps*/	111,	/*10 Bps*/	100,	/*11 Bps*/	91,		/*12 Bps*/	83,
	/*13 Bps*/	77,		/*14 Bps*/	71,		/*15 Bps*/	67,		/*16 Bps*/	63,
	/*17 Bps*/	59,		/*18 Bps*/	56,		/*19 Bps*/	53,		/*20 Bps*/	50,
	/*Unlimited*/ 0
  };
#if RAMP
//Pointer to math formulas for whatever Enhanced fire mode is being used
//ptrArray pMode;
void (* idata pMode) (void);

#define FORMULAS	(RAMP + 3)
code ptrArray MODE_ARRAY[FORMULAS] = {
	&SlowRamp,
	&MedRamp,
	&FastRamp,
	&HardRamp
	};	

unsigned char Carry;
#endif
/*
 *	Enhanced Firing process, loads extra Shot Buffers and sets ShotOffset as a multiple of TriggerROF
 *
*/
void
EF_PROCESS (void)
{
	if (!bShotReady)
		{
		if (SHOT_OFFSET_TIMER == 0)
			{
			bShotReady = 1;					// Let fire process know next shot is free to go
			}
		}

	// Check if we have request for Enhanced Fire
	if (TRIGGER_BUFFER && TRIGGER_COUNTED)
		{ //New one in buffer and it's been processed by Count_Pulls process
		if (SemiShots >= EFS_PULLS)
			{ // We have enough Pulls to meet requirement 1
#if RAMP			  
			if (EF_MODE == AUTO)
				{  //Set global so trigger function knows that continuous fire is enabled
				bAuto = 1;
				bMOD_EF = 0;
				bEfsState = 1;
				}
			else
#endif				
				{
				if (TriggerTics <= EFS_ROF_OFFSET)
					{ // Trigger ROF is high enough
#if RAMP
					if (EF_MODE > RESPONSE)
						{
						(*pMode)();
						if (SHOT_BUFFER > (SHOTVALUE * 3))
							{
								SHOT_BUFFER = (SHOTVALUE * 3);
							}
						if (TICS_BUCKET)
							{SHOT_OFFSET_TIMER = ShotOffset +1;}
						else
							{SHOT_OFFSET_TIMER = ShotOffset;}
						}
					else
						{ //Semi or Response, don't need to do anything special for Semi
						if (EF_MODE == RESPONSE)
							{//Set global bit so trigger function knows that Response is enabled
							bDouble = 1;
							}
						}
#endif
						bEfsState = 1;		//Set global flag indicating EFS PULLS and ROF met
					}
				else
					{ //Trigger ROF not high enough for EF, ShotOffset to max allowable
					ShotOffset = MAX_ROF_OFFSET;
					if (TICS_BUCKET)
						{SHOT_OFFSET_TIMER = ShotOffset + 1;}
					else
						{SHOT_OFFSET_TIMER = ShotOffset;}
					bDouble = 0;
					bEfsState = 0;		//Set global flag indicating below EFS PULLS and ROF
					}
				}
			}
		else
			{  // Not enough pulls for EF, ShotOffset to max allowable
			ShotOffset = MAX_ROF_OFFSET;
			bEfsState = 0;				//Set global flag indicating below EFS PULLS and ROF
			}
		TRIGGER_BUFFER = 0;			//Set back to 0 done with this shot			  
	}
}

#if RAMP  
/*
	SlowRamp = TriggerRof  + 50%, Max 1 extra shot in queue
*/  
void
SlowRamp (void)
{  // Divide by 2/3 + Carry, creates a Shot ROF of 1.5 * Trigger ROF
	Carry = TriggerTics % 3;
	ShotOffset = TriggerTics / 3; //Divide by 3
	ShotOffset = ShotOffset << 1; //Multiply by 2
	if (Carry > 0)
		{ShotOffset = ShotOffset + 1;} // And add the carry
	if (ShotOffset < MAX_ROF_OFFSET)
		{ShotOffset = MAX_ROF_OFFSET;}
	//Queue up another shot
	SHOT_BUFFER += (SHOTVALUE / 2);
}
/*
	MedRamp = TriggerRof + 100%, Max 1 extra shot in queue
*/

void
MedRamp (void)
  { // Divide by 2 + Carry, creates a Shot Rof of 2 * Trigger ROF
    Carry = TriggerTics % 2;
	ShotOffset = TriggerTics >> 1; //Divide by 2
	if (Carry > 0)
		{ShotOffset = ShotOffset + 1;} //Add the carry
	if (ShotOffset < MAX_ROF_OFFSET)
		{ShotOffset = MAX_ROF_OFFSET;}
	SHOT_BUFFER += SHOTVALUE;
  }
/*
	FastRamp = TriggerRof + 150%, Max 1 extra shot in queue
*/
void
FastRamp (void)
  { // multiply by 2/5 + carry, creates a Shot Rof of 2.5 * Trigger Rof
	Carry = TriggerTics % 5;
	ShotOffset = TriggerTics / 5;
	ShotOffset = ShotOffset << 1;
	if (Carry > 0)
		{ShotOffset = ShotOffset + 1;}
	if (ShotOffset < MAX_ROF_OFFSET) 
		{ShotOffset = MAX_ROF_OFFSET;}
	SHOT_BUFFER += (SHOTVALUE * 1.5);
  }
/*
	HardRamp = TriggerRof + 200%, Max 2 extra shots in queue
*/

void
HardRamp (void)
  { // Divice by 3 + carry, create a Shot Rof of 3 * Trigger Rof
	Carry = TriggerTics % 3;
	ShotOffset = TriggerTics / 3;
	if (Carry > 0)
		{ShotOffset = ShotOffset + 1;}
	if (ShotOffset < MAX_ROF_OFFSET)
		{ShotOffset = MAX_ROF_OFFSET;}
	SHOT_BUFFER += (SHOTVALUE * 2);
  }
#endif /* RAMP */
 