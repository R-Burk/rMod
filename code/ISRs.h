#ifndef ISRS_H
#define ISRS_H

#pragma callee_saves QRTR_RESET

void QRTR_SECOND 		(unsigned char trigmode);
void QRTR_RESET 		(void);
void CALC_DWELL			(void);
void TriggerISR 		(void) interrupt 0;// using 1;
void TicTimer0ISR		(void) interrupt 1;// using 1;
void Dwell1ISR			(void) interrupt 3;// using 2;
void TicTimer2ISR		(void) interrupt 5;// using 1;

// Global Tics counter, note this can never be reset or it would mess up every timer being used
extern volatile unsigned char TICS;
extern volatile unsigned char TICS_BUCKET;
extern volatile unsigned char TICS_BUFFER;
// Longer time tracks
extern unsigned char MINUTES;
extern unsigned char QRTRSEC;

//Timers that run based off TICS
extern unsigned char QRTR_TIMER;
//Dwell Variables
extern unsigned char DWELL_HIGH_ARS;
extern unsigned char DWELL_LOW_ARS;

#define RELOAD_TIC_SHORT 0x9C
#define RELOAD_TIC_HIGH_LONG 0xFC
#define RELOAD_TIC_LOW_LONG 0X18

#endif /* ISRS_H */
