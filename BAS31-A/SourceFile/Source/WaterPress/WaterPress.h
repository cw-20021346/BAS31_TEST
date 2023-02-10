#ifndef __WATERPRESS_H__
#define __WATERPRESS_H__

#include "prj_type.h"

#define DEFAULT_PRESS    1
#define WATER_PRESS_CHANGE_DELAY        50      /*10msec -> 500msec*/

#define PRESS_MINIUM     1
#define PRESS_MAXIUM     3

void InitWaterPress(void);

void SetWaterPressLevel( U8 mu8val);
U8 GetWaterPressLevel(void);

void SetWaterPressLevelTemp( U8 mu8val);
U8 GetWaterPressLevelTemp(void);

void SetWaterPressDelay( U16 mu16val);
U16 GetWaterPressDelay(void);

void SetWaterPressUserPress( U8 mu8val);
U8 GetWaterPressUserPress(void);
    

void Evt_10msec_Press_Handler(void);



#endif
