#ifndef RAMP_H
#define RAMP_H
void EF_PROCESS 		(void);
void SlowRamp			(void);
void MedRamp			(void);
void FastRamp			(void);
void HardRamp			(void);

#define SEMI		0
#define RESPONSE	1
#define SLOWRAMP	2
#define MEDRAMP		3
#define FASTRAMP	4
#define HARDRAMP	5
#define AUTO		6

extern bit bEfsState;	//EFS conditions active on current trigger pull
extern bit bDouble;	//Response Mode Active
extern bit bAuto;		//Full Auto Active
extern bit bMOD_EF;	//Sniper Mode State, 0 - Next pull is fire, 1 - Next pull is cycle bolt

#define PSP_NXL		1
#define BREAKOUT	2
#define SNIPER		3
#define MOD_EF_OFF	4		//Really 0 for code, value to show user in blinks

extern code const unsigned char ROF_ARRAY[];
typedef void (* ptrArray) (void);
extern code ptrArray MODE_ARRAY[];
//Pointer to math formulas for whatever Enhanced fire mode is being used	
//extern ptrArray pMode;
extern void (* idata pMode) (void);

//Timer to control shot ROF
extern unsigned char SHOT_OFFSET_TIMER;
//Global Ramp Variables
	//Semi shots made with less than 1 second between shots
extern unsigned char SemiShots;
	//Amount of time to be used for SHOT_OFFSET_TIMER;
extern unsigned char ShotOffset;

#endif
