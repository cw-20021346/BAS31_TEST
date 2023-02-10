#ifndef __PROCESS_DISPLAY_H__
#define __PROCESS_DISPLAY_H__

#include "prj_type.h"


void InitDisplay(void);

U8 GetDisplayInit(void);

    

U8 GetMementoDisp(void);
void SetMementoDisp(U8 mu8Conf );

void Evt_1msec_ProcessDisplay(void);
void Evt_100msec_ProcessDisplay(void);


#endif /* __PROCESS_DISPLAY_H__ */
