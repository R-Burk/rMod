#ifndef MEMORY_H
#define MEMORY_H
#pragma callee_saves WRITE_EEPROM,READ_EEPROM,RETURN_PTR

unsigned char* data RETURN_PTR(unsigned char slot);
void LOAD_PARAMS 		(void);
void LOAD_DEFAULTS 		(void);
void SELECT_SLOT 		(idata unsigned char* idata slotcount,idata unsigned char* idata ptrslotvalue);
void WRITE_EEPROM		(unsigned int address,unsigned char data);
unsigned char CheckEepromBusy	(void);
unsigned char READ_EEPROM(unsigned int address);
void PROGRAM_SLOT 		(unsigned char slot, idata unsigned char* idata pPull_Count);
void WRITE_SLOT			(unsigned char slot, unsigned char newvalue);
void aFailed			(unsigned char file, unsigned int location);
void PROGRAM_MODE		(void);

/* Global variables
	_EEADDRESS is the hex location in EEROM for the user programmed values
*/
//Enhanced Fire Mode
extern unsigned char EF_MODE;
#if FULLAUTO == 0
	#define EF_MODE_MAX						11
#else
	#define EF_MODE_MAX						12
#endif
//Modify Enhanced Fire
extern unsigned char MOD_EF;
//Enhanced firing start, ROF to hit before going into Enhanced firing mode
extern unsigned char EFS_ROF;
//Enhanced firing start, Pulls with less than 1 second between pulls before Enhanced firing can start
extern unsigned char EFS_PULLS;
//Eyes Operation Mode
extern unsigned char EYES_MODE;
//ROF Limit, eyes on
extern unsigned char EYES_MAX_ROF;
//ROF Limit, eyes off
extern unsigned char BYPASS_MAX_ROF;
//Globals associates with trigger Debounce
extern unsigned char DEBOUNCE;
//Slow Pull Debounce
extern unsigned char SLOW_DEBOUNCE;
//Cycle Inhibit Values
extern unsigned char CYCLE_INHIBIT;
//Milliseconds of on time for Solenoid 1
extern unsigned char DWELL_MAJOR;
// .1 of a millisecond additional on time for Solenoid 1
extern unsigned char DWELL_MINOR;
//Anti Ram Stick dwell, additional dwell after 15 seconds no firing
extern unsigned char ARS;
//Anti Ram Time, amount of time before ARS is added to Dwell
extern unsigned char ARS_TIME;
//Amount of time to wait from start of firing till powering up Solenoid 2
extern unsigned char BOLT_DELAY;	
//How long to debounce ball
extern unsigned char ACE_DEBOUNCE;		
//Max time to power Solenoid 2 and leave bolt open
extern unsigned char BOLT_OPEN;
#define EYES_BOLT_OPEN					125
//Max time to wait after releasing Solenoid 2 to close bolt
extern unsigned char BOLT_CLOSE;

#if PROGRAMMING
extern unsigned char ProgState;
//Trigger Programming States
#define TrigProg_WAITING		0
#define TrigProg_SELECTING		1
#define TrigProg_SHOWING		2
#define TrigProg_INPUTING		3
#define TrigProg_WRITING		4
#define TrigProg_RESTARTING 	5
#endif

#endif
