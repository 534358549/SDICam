#ifndef __HTX_3531A_H__
#define __HTX_3531A_H__

#include "sample_comm.h"

void initGlobalVar();

void releaseAllResource();

void checkSDIInput();

void startOSD();
void stopOSD();

HI_S32 startVIO(HI_VOID);
void stopVIO();

HI_S32 HTX_CreatThread(HI_VOID);
#endif