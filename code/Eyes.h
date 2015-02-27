#ifndef EYES_H
#define	EYES_H
void EYE_PROCESS 		(void);
void EYE_TOGGLE_CHECK 	(void);
void ACE_TEST			(void);

#define EYES_OFF		0	//ACE off, holding trigger down makes no difference
#define EYES_ALWAYS		1	//ACE on, holding trigger for .5 second will Force Shot in Viking Mode
#define EYES_NORM		2	//ACE on, trigger down for 5 seconds toggles ACE ON/OFF
#define EYES_DRYFIRE	3	//ACE on, Dwell is 0(Excal) or Inverted Ball Logic fire no ball/ no fire if ball (Viking)
#define EYES_TEST		4	//Special, EYE beam connected, LED on otherwise off

//Timer for Ball in Place count
extern unsigned char ACE_TIMER;

extern bit ACE_ENABLED;				//ACE shutdown by trigger
extern unsigned char ACE_STATE;
//Ace States
#define ACE_OFF					0
#define ACE_OPEN_BREAK			1	//Xmit on, Break will shift state to Open COnnect
#define ACE_OPEN_CONNECT		2	//Xmit on, Connect will shift state to Beam_Break
#define ACE_BREAK_ONCE			3
#define ACE_BEAM_BREAK			4 	//Xmit on, Break > Ace_Debounce will call BOLT_CLOSE_PROCESS
#define ACE_CLOSE_WAIT			5	//Xmit off, Timeout goes to CLOSE_BREAK
#define ACE_CLOSE_BREAK			6	//Xmit on, Break goes to CLOSE_CONNECT
#define ACE_CLOSE_CONNECT		7	//Xmit on, Connect will call BOLT_DONE_PROCESS 
#define ACE_NEW_STATE			128

#endif // END EYES_H
