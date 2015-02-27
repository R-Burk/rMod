#ifndef TRIGGER_H
#define TRIGGER_H

void POLL 				(void);
void COUNT_PULLS 		(void);
//unsigned char ComputeOffset (idata const unsigned char* idata current_,idata const unsigned char* idata previous_);
unsigned char ComputeOffset (unsigned char current_,unsigned char previous_);
void CYCLE_INHIBIT_PROCESS	(void);

// How long trigger must be stable to be considered debounced
extern unsigned char DEBOUNCE_TIMER;
// How long must pass since trigger pull before trigger can be reactivated
extern unsigned char CYCLE_INHIBIT_TIMER;
	//# of Tics from Previous to Current
extern unsigned char TriggerTics;

extern bit Trigger_Previous;
#define Trigger_Closed 	0
#define Trigger_Open	1
//Trigger bits
extern volatile bit TRIGGER_USER_REQUEST;		// Trigger request from user, set by ISR
extern bit TRIGGER_BUFFER;		// Buffer for trigger pull
extern bit TRIGGER_FIRING;		// Firing in process
extern bit TRIGGER_COUNTED;		// Current shot counted 
extern bit TRIGGER_INHIBIT;		// Trigger request being ignored

#endif
