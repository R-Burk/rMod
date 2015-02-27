/***********************************************************************
 *
 *   Functions associated with reading, writing, updating, values in EEPROM
 *
 ***********************************************************************/
#include "VaporB72.h"
#include "Memory.h"
#include "Led.h"
#include "CommonFiring.h"
#include "ISRs.h"
#include "Ramp.h"
#include "Extras.h"
#include "Trigger.h"

FILENUM(7) 
//Globals

/* Global variables
	_EEADDRESS is the hex location in EEROM for the user programmed values
*/
//Enhanced Fire Mode
unsigned char EF_MODE;
#define EF_MODE_SLOT						1
//EF_MODE_MAX in header

//Modify Enhanced Fire
unsigned char MOD_EF;
#define MOD_EF_SLOT							2
#define MOD_EF_MAX							4	// 4 is actually 0, 0 indicates off

//Enhanced firing start, ROF to hit before going into Enhanced firing mode
unsigned char EFS_ROF;
#define EFS_ROF_SLOT						3
#define EFS_ROF_MAX							17

//Enhanced firing start, Pulls with less than 1 second between pulls before Enhanced firing can start
unsigned char EFS_PULLS;
#define EFS_PULLS_SLOT						4
#define EFS_PULLS_MAX						16

//Eyes Operation Mode
unsigned char EYES_MODE;
#define EYES_MODE_SLOT						5
#define EYES_MODE_MAX						5

//ROF Limit, eyes on
unsigned char EYES_MAX_ROF;
#define EYES_MAX_ROF_SLOT					6
#define EYES_MAX_ROF_MAX					18	//18 + 3, Puts you in slot 21 of ROF_Array

//ROF Limit, eyes off
unsigned char BYPASS_MAX_ROF;
#define BYPASS_MAX_ROF_SLOT					7
#define BYPASS_MAX_ROF_MAX					17 //Same as EYES_MAX_ROF

//Globals associates with trigger Debounce
unsigned char DEBOUNCE;
#define DEBOUNCE_SLOT						8
#define DEBOUNCE_MAX						30

//Slow Pull Debounce
unsigned char SLOW_DEBOUNCE;
#define SLOW_DEBOUNCE_SLOT					9
#define SLOW_DEBOUNCE_MAX					10

//Cycle Inhibit Values
unsigned char CYCLE_INHIBIT;
#define CYCLE_INHIBIT_SLOT					10
#define CYCLE_INHIBIT_MAX					21  // Decremented by 1 when saved, 0 is off to code, *2 when brought in, each pull is 2

//Milliseconds of on time for Solenoid 1
unsigned char DWELL_MAJOR = 0;
#define DWELL_MAJOR_SLOT					11
#define DWELL_MAJOR_MAX						24

// .1 of a millisecond additional on time for Solenoid 1
unsigned char DWELL_MINOR = 0;
#define DWELL_MINOR_SLOT					12
#define DWELL_MINOR_MAX						10 //Note will be decremented by 1 when saved to memory, so range is 0 - 9, so 1 pull is off

//Anti Ram Stick dwell, additional dwell after 15 seconds no firing
unsigned char ARS;
#define ARS_SLOT							13
#define ARS_MAX								10

//Anti Ram Time, amount of time before ARS is added to Dwell
unsigned char ARS_TIME;
#define ARS_TIME_SLOT						14
#define ARS_TIME_MAX						4  //Note 0 is off, 1 is 15 seconds, 2 is 30 seconds, 3 is 45 seconds, Decremented when saved

//Amount of time to wait from start of firing till powering up Solenoid 2
unsigned char BOLT_DELAY ;	
#define BOLT_DELAY_SLOT						15
#define BOLT_DELAY_MAX						25

//How long to debounce ball
unsigned char ACE_DEBOUNCE;
#define ACE_DEBOUNCE_SLOT					16
#define ACE_DEBOUNCE_MAX					20

//Max time to power Solenoid 2 and leave bolt open
unsigned char BOLT_OPEN;
#define BOLT_OPEN_SLOT						17
#define BOLT_OPEN_MAX						62	// Note pull is *4 when saved ( saves pulling forever)

//Max time to wait after releasing Solenoid 2 to close bolt
unsigned char BOLT_CLOSE;
#define BOLT_CLOSE_SLOT						18
#define BOLT_CLOSE_MAX						40	

//Total number of slots
#define SLOTS_MAX							18

code const unsigned char EXCAL_DEFAULT_ARRAY[SLOTS_MAX+1] =   {0,SEMI    ,0,4,3,1,21,13,5,5, 1,10,0,2,1, 2,4,56,26};
code const unsigned char OPEN_BOLT_DEFAULT_ARRAY[SLOTS_MAX+1]={0,SEMI    ,0,4,3,1,21,13,5,5, 1,17,0,2,1, 1,4, 4, 1};

//Array stores the memory address holding value for corresponding variable in VAR_PTR_ARRAY
#define ROOT_EEADDRESS					0x030
#define EEADDRESS(num) (ROOT_EEADDRESS + ((num-1) * 3))
code const unsigned int EEPROM_REF_ARRAY[SLOTS_MAX+1] = {
	0,				//Placeholder
	EEADDRESS(1),
	EEADDRESS(2),
	EEADDRESS(3),
	EEADDRESS(4),
	EEADDRESS(5),
	EEADDRESS(6),
	EEADDRESS(7),
	EEADDRESS(8),
	EEADDRESS(9),
	EEADDRESS(10),
	EEADDRESS(11),
	EEADDRESS(12),
	EEADDRESS(13),
	EEADDRESS(14),
	EEADDRESS(15),
	EEADDRESS(16),
	EEADDRESS(17),
	EEADDRESS(18)
 };
	
//Array holds the max value allowed to be programmed in for corresponding variable in VAR_PTR_ARRAY	
code const unsigned char SLOT_MAX_ARRAY[SLOTS_MAX+1] = {
	0,		//Placeholder
	EF_MODE_MAX,
	MOD_EF_MAX,
	EFS_ROF_MAX,
	EFS_PULLS_MAX,
	EYES_MODE_MAX,
	EYES_MAX_ROF_MAX,
	BYPASS_MAX_ROF_MAX,
	DEBOUNCE_MAX,
	SLOW_DEBOUNCE_MAX,
	CYCLE_INHIBIT_MAX,
	DWELL_MAJOR_MAX,
	DWELL_MINOR_MAX,
	ARS_MAX,
	ARS_TIME_MAX,
	BOLT_DELAY_MAX,
	ACE_DEBOUNCE_MAX,
	BOLT_OPEN_MAX,
	BOLT_CLOSE_MAX
	};

unsigned char* data
RETURN_PTR (unsigned char slot)
  {
	data unsigned char * data ptr = &DEBOUNCE;
	switch(slot)
	  { //find case match, put address into ptr
		case(EF_MODE_SLOT):
			ptr = &EF_MODE;
			break;
		case(MOD_EF_SLOT):
			ptr = &MOD_EF;
			break;
		case(EFS_ROF_SLOT):
			ptr = &EFS_ROF;
			break;
		case(EFS_PULLS_SLOT):
			ptr = &EFS_PULLS;
			break;
		case(EYES_MODE_SLOT):
			ptr = &EYES_MODE;
			break;
		case(EYES_MAX_ROF_SLOT):
			ptr = &EYES_MAX_ROF;
			break;
		case(BYPASS_MAX_ROF_SLOT):
			ptr = &BYPASS_MAX_ROF;
			break;
		case(DEBOUNCE_SLOT):
			ptr = &DEBOUNCE;
			break;
		case(SLOW_DEBOUNCE_SLOT):
			ptr = &SLOW_DEBOUNCE;
			break;
		case(CYCLE_INHIBIT_SLOT):
			ptr = &CYCLE_INHIBIT;
			break;
		case(DWELL_MAJOR_SLOT):
			ptr = &DWELL_MAJOR;
			break;
		case(DWELL_MINOR_SLOT):
			ptr = &DWELL_MINOR;
			break;
		case(ARS_SLOT):
			ptr = &ARS;
			break;
		case(ARS_TIME_SLOT):
			ptr = &ARS_TIME;
			break;
		case(BOLT_DELAY_SLOT):
			ptr = &BOLT_DELAY;
			break;
		case(ACE_DEBOUNCE_SLOT):
			ptr = &ACE_DEBOUNCE;
			break;
		case(BOLT_OPEN_SLOT):
			ptr = &BOLT_OPEN;
			break;
		case(BOLT_CLOSE_SLOT):
			ptr = &BOLT_CLOSE;
			break;
	  }
	//global variable set equal to variable reference from calling function
	return ptr;
  }
  
void
LOAD_PARAMS (void)
  {
	unsigned char 	SLOT_VAR = 0xAA;
	unsigned char 	*pSLOT_VAR;
	unsigned int 	EEADDRESS;
	unsigned char 	EEMEM_1;
	unsigned char 	EEMEM_2;
	unsigned char 	EEMEM_3;
	unsigned char 	i;
	
	pSLOT_VAR = &SLOT_VAR;
	
	//Start at top slot number and decrement down to 0
	for(i=SLOTS_MAX;i;i--)
	  {
		EEADDRESS = (unsigned int) EEPROM_REF_ARRAY[i];
		pSLOT_VAR = RETURN_PTR(i);
		for(;;)
		  {
			EEMEM_1 = READ_EEPROM((unsigned int)EEADDRESS);
			EEMEM_2 = READ_EEPROM((unsigned int)(EEADDRESS+1));
			EEMEM_3 = READ_EEPROM((unsigned int)(EEADDRESS+2));
			//Check that all 3 are good
			if ((EEMEM_1 == EEMEM_2) && (EEMEM_2 == EEMEM_3))
				  { //All good
					*pSLOT_VAR = EEMEM_1;
					if (*pSLOT_VAR == 0xFF)
					  {
						LOAD_DEFAULTS();
					  }
					else
					  {
						break;
					  }
				  }
			else
			  { // One is bad, find and rewrite
				if (EEMEM_2 == EEMEM_3)
				  { //Rewrite mem1
					WRITE_EEPROM((unsigned int)EEADDRESS,(unsigned char)EEMEM_2);
				  }
				if (EEMEM_1 == EEMEM_3)
				  { //Rewrite mem2
					WRITE_EEPROM((unsigned int)(EEADDRESS+1),(unsigned char)EEMEM_1);
				  }
				//Check mem3 bad
				if (EEMEM_1 == EEMEM_2)
				  { //Rewrite mem3
					WRITE_EEPROM((unsigned int)(EEADDRESS+2),(unsigned char)EEMEM_1);
				  }
			  }
		  }
	  }
  }
  
void
LOAD_DEFAULTS (void)
  { 
	unsigned char DEFAULT_VAR;
	unsigned char SLOT_VAR;
    unsigned char* data pSLOT_VAR;
	unsigned int 	EEADDRESS;
	unsigned char i;// = 1;
	
	pSLOT_VAR = &SLOT_VAR;

	for(i=SLOTS_MAX;i;i--)
	  {
		LED != LED;
		pSLOT_VAR = RETURN_PTR(i);					//Set pointer to global variable for slot i
#if OPEN_BOLT && CLOSE_BOLT
		if (VIK_EXCAL_DIP1 == 0)
		  {
#endif
  #if CLOSE_BOLT		  
			DEFAULT_VAR = EXCAL_DEFAULT_ARRAY[i];	//Load Default with value from HUMP_DEFAULT_ARRAY
  #endif
#if OPEN_BOLT && CLOSE_BOLT
		  }
		else
		  {
#endif
  #if OPEN_BOLT
			DEFAULT_VAR = OPEN_BOLT_DEFAULT_ARRAY[i];	//Load Default with value from MAC_DEFAULT_ARRAY
  #endif
#if OPEN_BOLT && CLOSE_BOLT
		  }
#endif

		EEADDRESS = EEPROM_REF_ARRAY[i];				//Load EEProm address for current slot
		WRITE_EEPROM(EEADDRESS, DEFAULT_VAR);		//Write value to EEProm and next 2 addresses
		WRITE_EEPROM(EEADDRESS++, DEFAULT_VAR);		
		WRITE_EEPROM(EEADDRESS++, DEFAULT_VAR);		
		*pSLOT_VAR = DEFAULT_VAR;					//Update global variable in memory
	  }
	
#if SHOWVER || VLINK
	WRITE_EEPROM(VLINK_TIMER_MINOR, CurrentBuild); 	//Timer Time Minor - Vlink multiplies by 5 for display - BestBps
#endif
#if SHOWVER || VLINK
	WRITE_EEPROM(VLINK_VER_MAJOR, VersionMajor); 	//Firmware Version Major
	WRITE_EEPROM(VLINK_VER_MINOR, VersionMinor); 	//Firmware Version Minor
  #if PANDORA
	WRITE_EEPROM(VLINK_BOARD_TYPE, 2);  	//Board Type- 3 Loki, 2 Pandora, 1 Entropy, 0 Chaos
  #else
	WRITE_EEPROM(VLINK_BOARD_TYPE, 0);
  #endif
#endif
#if VLINK
	WRITE_EEPROM(VLINK_AE_BYPASS_ROF, 255); 	//AE Bypass ROF
	WRITE_EEPROM(VLINK_LCD8_MENU_A, 255);	//First 8 LCD Menu Options
	WRITE_EEPROM(VLINK_LCD8_MENU_B, 255);	//Second 8 LCD Menu Options
	WRITE_EEPROM(25, 255);	//No CLue
#endif
#if VLINK
	WRITE_EEPROM(27, 255);
	WRITE_EEPROM(28, 255);
#endif
#if SHOWVER || VLINK
	WRITE_EEPROM(VLINK_BATT_MINOR, 0); 	//Battery Minor, divided by 255 for 1/10th value	
	WRITE_EEPROM(VLINK_BATT_MAJOR, 8); 	//Battery Major
  #if OPEN_BOLT && CLOSE_BOLT
	WRITE_EEPROM(VLINK_BOLT_MODE, !VIK_EXCAL_DIP1);  	//0 Open Bolt, 1 Closed Bolt
  #elif OPEN_BOLT
	WRITE_EEPROM(VLINK_BOLT_MODE, 0);
  #else
	WRITE_EEPROM(VLINK_BOLT_MODE, 1);
  #endif
  #if VLINK
	WRITE_EEPROM(32, 255);
	WRITE_EEPROM(33, 255);
	WRITE_EEPROM(34, 255);
	WRITE_EEPROM(35, 255);
  #endif
#endif
	LIGHT = LIGHT_OFF + LIGHT_NEW_STATE;
  }
/***********************************************************************
 *                    
 *  FUNCTION     :  SELECT_SLOT()
 *  DESCRIPTION  :  Interprets trigger pulls as register being selected
 *  PARAMETERS   :  unsigned char* - address of variable holding slot number.
 *					unsigned char* - address of variable to be loaded with value in slot
 *  RETURNS      :  N/A
 *
 ***********************************************************************/
 #if PROGRAMMING
void
SELECT_SLOT (idata unsigned char * idata slotcount,idata unsigned char* idata ptrslotvalue)
  {
	unsigned char EEADDRESS;
    // Check for a request
	if (SHOT_BUFFER)
	  { 
		//Clear request
		SHOT_BUFFER = 0;
		(*slotcount)++;
		//Check for too high and reset to 0
		if (((*slotcount) > SLOTS_MAX)) 
		  {
			(*slotcount) = 0;
			//Give them a fast blink to let know the counter has rolled over
			LIGHT = LIGHT_FAST_ONCE + LIGHT_NEW_STATE;
		  }
		else
		  {
			//Show user a blink that trigger was read
			LIGHT = LIGHT_BLINK_ONCE + LIGHT_NEW_STATE;
		  }	
		//Return # of blinks that will be required for current slot
		EEADDRESS = EEPROM_REF_ARRAY[(*slotcount)];
		*ptrslotvalue = READ_EEPROM(EEADDRESS);
		
		//Values below need to be adjusted when shown by LED to make sense to User
		//Dwell_Minor  need a real range of 0 -9
		if ((*slotcount) == DWELL_MINOR_SLOT) {(*ptrslotvalue)++;}
		// ARS_TIME need a real range of 0 - 3 ( 0 indicates not in use)
		if ((*slotcount) == ARS_TIME_SLOT) {(*ptrslotvalue)++;}
		// Mode Slot, 0 indicating semi mode, need to decrement save value
		if ((*slotcount) == EF_MODE_SLOT) {(*ptrslotvalue)++;}
		// EF MOde 2 Slot, O indicates off
		if ((*slotcount) == MOD_EF_SLOT)	{
			if ((*ptrslotvalue) == 0) {(*ptrslotvalue) = MOD_EF_OFF;}
			}
		// Cycle Inhibit, 0 indicates it is off to code
		if ((*slotcount) == CYCLE_INHIBIT_SLOT) {(*ptrslotvalue)++;}
		// Eyes Mode, 0 indicates ACE is off
		if ((*slotcount) == EYES_MODE_SLOT) {(*ptrslotvalue)++;} 
		if ((*slotcount) == BOLT_OPEN_SLOT) {(*ptrslotvalue) = (*ptrslotvalue) >> 2;} //2 bit shift or divide by 4
		if ((*slotcount) == EYES_MAX_ROF_SLOT) {(*ptrslotvalue) = (*ptrslotvalue) - 3;} //Decrement - 3, min I allow is 4, 4 - 3 is 1 blink
		if ((*slotcount) == BYPASS_MAX_ROF_SLOT) {(*ptrslotvalue) = (*ptrslotvalue) - 3;} //Same as reason above
		QRTR_RESET();		
	  }
	  
	// 3 seconds since last trigger pull ,check if something to do
	if (QRTRSEC == 12)
	  {
		if ((*slotcount) > 0)
		  { // Set to next state
			ProgState = TrigProg_SHOWING;
			QRTR_RESET();
		  }
		else
		  { // Set back to start of process
			ProgState = TrigProg_WAITING;
		  }
	  }
  }

/***********************************************************************
 *                    
 *  FUNCTION     :  PROGRAM_SLOT()
 *  DESCRIPTION  :  Interprets trigger pulls as value to be loaded into slot
 *  PARAMETERS   :  unsigned char - slot selected for updating
 *					unsigned char* - address of variable recording trigger pulls
 *  RETURNS      :  N/A
 *
 ***********************************************************************/
#endif
#if PROGRAMMING
void  
PROGRAM_SLOT (unsigned char slot,idata unsigned char* idata pPull_Count)
  {
	unsigned char MAX_FOR_SLOT;

	MAX_FOR_SLOT = SLOT_MAX_ARRAY[slot];
	
	if (SHOT_BUFFER)
	  {
		(*pPull_Count)++;
		SHOT_BUFFER = 0;
		if ((*pPull_Count) > MAX_FOR_SLOT) 
		  { //Reset to 0 and fast blink to let user know counter is at 0
			(*pPull_Count) = 0;
			LIGHT = LIGHT_FAST_ONCE + LIGHT_NEW_STATE;
			QRTR_RESET();
		  }
		else
		  { //Full blink to acknowledge pull
			LIGHT = LIGHT_BLINK_ONCE + LIGHT_NEW_STATE;
			QRTR_RESET();
		  }
	  }
	// Trigger not pressed for 3 seconds, progress to next state
	if (QRTRSEC == 12)
	  {
		if (*pPull_Count > 0)
		  { // If something to write we go to next state
			ProgState = TrigProg_WRITING;
		  }
		else
		  { // Guess user was just looking go back to state 0
			LIGHT = LIGHT_FAST_ONCE + LIGHT_NEW_STATE;
			ProgState = TrigProg_WAITING;
		  }
	  }
  }
  
/***********************************************************************
 *                    
 *  FUNCTION     :  WRITE_SLOT()
 *  DESCRIPTION  :  Updates EEPROM location associated with slot with slotvalue
 *  PARAMETERS   :  unsigned char - slot selected for updating
 *					unsigned char - value to write into EEPROM for slot
 *  RETURNS      :  N/A
 *
 ***********************************************************************/
void
WRITE_SLOT (unsigned char slot,unsigned char newvalue)
{
	unsigned char ValToWrite;
	unsigned int EEADDRESS;
	unsigned char CHECKSUM;
	
	ValToWrite = newvalue;
	  
	EEADDRESS = EEPROM_REF_ARRAY[slot];
	
	//Values below need 1 to be able to enter write_slot, need to decrement to make sense in code
	//Dwell_Minor  need a real range of 0 -9
	if (slot == DWELL_MINOR_SLOT) 	{ValToWrite--;}
	// ARS_TIME need a real range of 0 - 3 ( 0 indicates not in use)
	if (slot == ARS_TIME_SLOT) 	{ValToWrite--;}
	// Mode Slot, 0 indicating semi mode, need to decrement save value
	if (slot == EF_MODE_SLOT) 		{ValToWrite--;}
	// EF Mode Slot 2, 0 indicates off
	if (slot == MOD_EF_SLOT) {
		if (ValToWrite == MOD_EF_OFF)	{ValToWrite = 0;}
		}
	// Cycle Inhbit, 0 indicates to code not in use
	if (slot == CYCLE_INHIBIT_SLOT) {ValToWrite--;}
	// Eyes Mode, 0 indicates ACE is not being used
	if (slot == EYES_MODE_SLOT) 	{ValToWrite--;} 
	
	//Special slot, because it's such a high value each pull is actually 4
	if (slot == BOLT_OPEN_SLOT) 	{ValToWrite = ValToWrite << 2;} // 2 bit shift left or *4
	
	if (slot == EYES_MAX_ROF_SLOT) 		{ValToWrite = ValToWrite + 3;} //Incrementing up, so 1 pull is 4 rof and 18 pull is 21(Unlimited)
	if (slot == BYPASS_MAX_ROF_SLOT) 	{ValToWrite = ValToWrite + 3;} //Same as reason above
	
	EA = 0;	//Shutting down interrupts, forces WRITE_EEPROM to block and wait on the write process to finish
	for(;;)
		{
		//Compute a checksum which is the sum of the Value_Counter * 3
		CHECKSUM = (unsigned char)(0xFF - (unsigned char)(ValToWrite*3));
		//Write the value to 3 contiguous memory positions
		WRITE_EEPROM(EEADDRESS  , ValToWrite);
		WRITE_EEPROM(++EEADDRESS, ValToWrite);
		WRITE_EEPROM(++EEADDRESS, ValToWrite);
		//Add the 3 writes of values stored in SLOT_PTR to SLOT_PTR +3
		CHECKSUM += READ_EEPROM(EEADDRESS);
		CHECKSUM += READ_EEPROM(--EEADDRESS);
		CHECKSUM += READ_EEPROM(--EEADDRESS);
		//Make sure we are back to 255 in Checksum otherwise Loop
		if (CHECKSUM = 0xFF) 
			{
			break;
			}
		}
	EA = 1;
	TRIGGER_USER_REQUEST = 0;
	SHOT_BUFFER = 0;
	QRTR_RESET();
}
 #endif
 
#if PRODUCTION == 0
void
aFailed (unsigned char file, unsigned int location)
  {
	unsigned char temp;
	WRITE_EEPROM(30, file); //File information into Battery voltage Major part
	temp = (unsigned char)location;
	WRITE_EEPROM(3, temp);
	temp = (unsigned char)(location >> 8);
	WRITE_EEPROM(2, temp);
#if DEBUG == 1
	for(;;)
	  {
		;
		;
		PCON |= PD;		//Chip Power down
		;
		;
	  }
#endif	  
  }
#endif

/***********************************************************************
 *                    
 *  FUNCTION     :  WRITE_EEPROM()
 *  DESCRIPTION  :  Save 1 byte to EEPROM at AT89S8252.
 *  PARAMETERS   :  unsigned char - data character to save.
 *					unsigned int  - Address of EEPROM
 *  RETURNS      :  N/A
 *
 ***********************************************************************/
void
WRITE_EEPROM (unsigned int Address, unsigned char Data)
{
	xdata unsigned char * data XEEPData;

	XEEPData=(xdata unsigned int*) Address;
	WMCON |= WMCON_EEMEN;			// Enable EEPROM
	WMCON |= WMCON_EEMWE;			// Enable Write
	*XEEPData=Data;					// Go Write to EEPROM
	if (EA == 0)
		{
		while (CheckEepromBusy())
			{ ;	}							// wait busy bit
		}
}
unsigned char
CheckEepromBusy	(void)
{
	if (!(WMCON & WMCON_WDTRST))
		{ //Busy is true, so returning that
		return 1;
		}
	else
		{ //Busy is done, 
		WMCON &= 0x77;		//Disabled EEPROM Write and Access
		return 0;
		}
}
/***********************************************************************
 *                    
 *  FUNCTION     :  READ_EEPROM()
 *  DESCRIPTION  :  Read 1 byte from EEPROM.
 *  PARAMETERS   :  unsigned int  - Address of EEPROM
 *  RETURNS      :  unsigned char - Data
 *
 ***********************************************************************/
unsigned char 
READ_EEPROM (unsigned int Address)
  {
	xdata unsigned char * data XEEPData;
	unsigned char Data;

	XEEPData=(xdata *) Address;		
	WMCON |= WMCON_EEMEN;			// EEPROM Enable
	Data = *XEEPData;				// Read Now
	WMCON &= 0xF7;					// Disable MOVX EEPROM
	return Data;
  }

#if PROGRAMMING

#pragma save
#pragma overlay
unsigned char ProgState;

void
PROGRAM_MODE (void)
  {
	//Variable and pointer for slot selection
	unsigned char   SLOTCOUNT = 0;
	idata unsigned char* idata pSLOTCOUNT;
	//Variable and pointer for tracking # of blinks while displaying current value in slot
	unsigned char   SLOTBLINKS = 0;
	idata unsigned char* idata pSLOTBLINKS;
	//Variable and pointer for slot value selection
	unsigned char   NEW_VALUE = 0;
	idata unsigned char* idata pNEW_VALUE;

	
	pSLOTCOUNT = &SLOTCOUNT;
	pSLOTBLINKS = &SLOTBLINKS;
	pNEW_VALUE = &NEW_VALUE;
	LED = 0;
	
	while(!Trigger)
	  {
		QRTR_SECOND(1);
		// Hold trigger for 10 seconds, load Mac defaults automatically.
		if (QRTRSEC == 40)
			{LOAD_DEFAULTS();}
		TICS = TICS_BUFFER;
	  }
	
	QRTR_RESET();
	ProgState = TrigProg_WAITING;
	LIGHT = LIGHT_BLINK_ONCE + LIGHT_NEW_STATE;
	
	for(;;)
	  {
		bAuto = 0;
		bDouble = 0;
		DEBOUNCE = 50;
		QRTR_SECOND(1);
		POLL();
		if (TRIGGER_USER_REQUEST)				
		  {
			//Toggling instead of setting, in case ISR toggles, then I still catch all trigger pulls
			TRIGGER_USER_REQUEST = 0;
			//Move Trigger request to buffer, allows only one in buffer by doing this
			SHOT_BUFFER = 1;
			//Restart long time counter process to 0
			QRTR_RESET();
		  }		
		// State 0
		if (ProgState == TrigProg_WAITING) 
		  {
			SLOTCOUNT = 0;
			NEW_VALUE = 0;
			SLOTBLINKS = 0;
			if (SHOT_BUFFER)
			  {
				ProgState = TrigProg_SELECTING;
			  }
		  }
		// State 1
		if (ProgState == TrigProg_SELECTING) {
			SELECT_SLOT(pSLOTCOUNT, pSLOTBLINKS); }
		// State 2
		if (ProgState == TrigProg_SHOWING)
		  {
			LED_READ_OUT(SLOTBLINKS);
			ProgState = TrigProg_INPUTING; //Move state to next in process
		  }
		// State 3
		if (ProgState == TrigProg_INPUTING) {
			PROGRAM_SLOT(SLOTCOUNT, pNEW_VALUE);}
		// State 4
		if (ProgState == TrigProg_WRITING)
		  {
			WRITE_SLOT(SLOTCOUNT, NEW_VALUE); 
			LED_READ_OUT(2);
			ProgState = TrigProg_WAITING;
		  }
		if (TICS != TICS_BUFFER)
			{
			TICS = TICS_BUFFER;
			LED_CONTROL();		// Controls Led States and Blink rates
			}
	  }
  }
#pragma restore
#endif
