#ifndef LED_H
#define LED_H
void LED_CONTROL 		(void);
void LED_READ_OUT		(unsigned char Blinks);
void LedByBeamState		(void);

// Basically does PWM for LED
extern unsigned char LED_TIMER;

extern unsigned char LIGHT;
//Light States
#define LIGHT_OFF 			0		
#define LIGHT_SLOW			1
#define LIGHT_FAST			2			
#define LIGHT_SOLID_DIM		3
#define LIGHT_SOLID_BRIGHT	4
#define LIGHT_BLINK_ONCE	5			//Use to acknowledge trigger pull during trigger programming
#define LIGHT_FAST_ONCE		6			//Use to Indicate counter rolled to 0 in trigger programming
#define LIGHT_NEW_STATE		128			// Indicates state shift request. Need to reload timer and reset LED.

extern unsigned char BEAM;
//Beam States - using order of power for bits so I can right shift
#define BEAM_GOOD			1	//Eyes saw ball overrode timer and loaded
#define BEAM_Failed_1		2	//Receiver never saw the Transmitter at all
#define BEAM_Failed_2		4	//Receiver had no signal for a 2nd time
#define BEAM_SoftOverRide	8	//Receiver had no signal for 3rd time, timers shifted to Bypass but still looking for beam to return
#define BEAM_HardOverRide	16	//Trigger disable of eyes
#define BEAM_NEW_STATE		128

#endif
