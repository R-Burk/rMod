#ifndef PANIO_H
#define PANIO_H

#include <AT89S8252.h>

// I/O mappings
#define Trigger 			INT0	//TTL Logic, 1 is off, 0 is on
#define ACE_XMIT 			INT1	//TTL Logic
#define ACE_RECV			P3_7	//TTL Logic
#define LED 				P3_4	//TTL Logic
#if PANDORA
  #define SOLENOID1 			P2_4	//Solenoids are not on inverted TTL logic. 0 is off, 1 is on
#endif
#define SOLENOID2 			P3_5
#if PANDORA && CLOSE_BOLT && OPEN_BOLT
  #define VIK_EXCAL_DIP1 		P2_6	//Tourney Lock
#endif
#define TOURNEY_DIP2		P1_2	//Excal/Viking DIP, Defaults Selector
//#define INTELLIFEED				// Tied to switch trigger, sheesh

#if PANDORA
	#define SOLENOID_ON		1
	#define SOLENOID_OFF	0
#else
	#define SOLENOID_ON		0
	#define SOLENOID_OFF	1
#endif

//Vlink Memory Positions
#define VLINK_FIRING_MODE	0
#define VLINK_ROF			1
#define VLINK_DWELL			2	//Does 3 digits
#define VLINK_DEBOUNCE		3	//Does 3 digits - 1 divided by 2
#define VLINK_AE_DELAY		4	//Does 2 digits
#define VLINK_TIMER_MODE	5
#define VLINK_TIMER_INIT	6
#define VLINK_TIMER_MAJOR	7	//Only goes to 15
#define VLINK_TIMER_MINOR	8	//Multiplies by 5 when displayed in Vlink
#define VLINK_LCD_1			9
#define VLINK_LCD_1B		10
#define VLINK_LCD_2			11
#define VLINK_LCD_2B		12
#define VLINK_EYE_MODE		13	//0 is drop
#define VLINK_OPER_MODE		14	//0 is standard
#define VLINK_SCOPE			15	//0 is Mode A, 1 is Mode B
#define VLINK_BOLT_DELAY	16	//only shows in closed bolt mode
#define VLINK_MIN_BOLT		17	//Only shows in closed bolt mode
#define VLINK_AFTER_SHOT	18	//Only shows in closed bolt mode
#define VLINK_VER_MAJOR		19	//Only will work for 2, 3, 4 - Major & Minor less than 3.20 will cause popups
#define VLINK_VER_MINOR		20	
#define VLINK_BOARD_TYPE	21	//Board Type- 3 Loki, 2 Pandora, 1 Entropy, 0 Chaos
#define VLINK_AE_BYPASS_ROF	22
#define VLINK_LCD8_MENU_A	23
#define VLINK_LCD8_MENU_B	24
#define VLINK_POWERSAVE		26
#define VLINK_BATT_MINOR	29	//divided by 255 for 1/10th value
#define VLINK_BATT_MAJOR	30	//Battery Major & Minor less than 6.5 will cause popups
#define VLINK_BOLT_MODE		31	//0 Open Bolt, 1 Closed Bolt

#endif
