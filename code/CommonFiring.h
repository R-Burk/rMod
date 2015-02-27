#ifndef __COMMON_FIRING_H__
#define __COMMON_FIRING_H__

void FIRECHECK 		(void);
void FIRE_SHOT		(void);

extern unsigned char BOLT_TIMER;
extern unsigned char BOLT_BUCKET;
	//Current MAX_OFFSET for ROF
extern unsigned char MAX_ROF_OFFSET;
	//Offset for EFS start
extern unsigned char EFS_ROF_OFFSET;

extern bit bBallReady;		// State of ball in breach in Viking mode
extern bit bForceShot;		// ForceShot Request Buffer
extern bit bForceDone;		// Note a ForceShot has been done this trigger pull
// Shot buffers
extern bit bShotReady;		// Timer (EFS_TIMER) has expired allowing another shot to be fired if ball is in place
extern unsigned char SHOT_BUFFER;  	// Shot queuing buffer

#define SHOTVALUE	64

//Bolt States
extern unsigned char BOLT_STATE;
#define BOLT_OFF_FLAG			0	//Start of Bolt Cycle Process
#define BOLT_DELAY_FLAG 		1	//Bolt Process is waiting
#define BOLT_OPEN_FLAG			2	//Bolt Process is moving bolt open
#define BOLT_CLOSE_FLAG 		4	//BOLT Process is moving bolt closed
#define BOLT_VIK_REQ_FLAG		8	//Viking Shot has been requested and Bolt Timer is counting
#define BOLT_VIK_START_FLAG		16	//Viking Bolt Process is starting
#define BOLT_VIK_OPEN_FLAG		32  //Viking Bolt is moving open

#endif
