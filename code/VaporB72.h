#ifndef __VAPORB7_H__
#define __VAPORB7_H__

/*
	This file contains the globals values used by all the sub modules in this project
	Needs to be included first to allow globals to be read in before other includes
*/

#define VersionMajor 4
#define VersionMinor 20
#define CurrentBuild 3

#define PRODUCTION	1
	//0 is off, 1 is hardware, 2 is Simulator
#define POWERSAVE	1
#define PROGRAMMING	1

#define CHAOS		0
#define PANDORA		1
#define CLOSE_BOLT	1
#define OPEN_BOLT	1

#define CYCLIC		1
#define RAMP		1
#define SHOWVER		1
#define VLINK		0

#define FULLAUTO	1

#define DEBUG		0

//Macros
#if PRODUCTION
	#define FILENUM(NUM)
	#define ASSERT(expr)
#else
	#define FILENUM(num) \
		enum { F_NUM=num }; \
		void _dummy##num(void) _naked {}
	
	#define ASSERT(expr) \
		if (expr) \
			{} \
		else \
			aFailed(F_NUM, __LINE__) 
#endif

#define NOP _asm NOP _endasm

/*	_dummy list
	1	VaporB5
	2	CloseBolt
	3	Extras
	4	Eyes
	5	ISRs
	6	Led
	7	Memory
	8	Ramp
	9	Trigger
	10	OpenBolt
	11	CommonFiring
*/

//Files needed by anything in this project
#include <AT89S8252.h>
#include "PanIo.h"

#endif /* End __VAPORB5_H__ */
