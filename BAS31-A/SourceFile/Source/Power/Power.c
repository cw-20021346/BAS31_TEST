#include "Power.h"


typedef struct _power_
{
    U8 OnOff;
    
} Power_T;

Power_T Power;

void InitPower ( void )
{
    Power.OnOff = ON;
}

void SetPowerOnOff ( U8 mu8Val)
{
    Power.OnOff = mu8Val;
}

U8 GetPowerOnOff ( void )
{
    return Power.OnOff;
}

