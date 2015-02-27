#ifndef __EXTRAS_H__
#define __EXTRAS_H__

void IDLE				(void);
void SIM_IDLE			(void);
void POWER_DOWN			(void);

unsigned char ComputeBuckets (idata unsigned char* idata NumToSub,unsigned char current_,unsigned char previous_);

#if POWERSAVE == 2
extern bit bSimIdle;
#endif

#endif /* End of __EXTRAS_H__ */
