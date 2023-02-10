#ifndef __KEYPBA_H__
#define __KEYPBA_H__

#include "prj_type.h"

typedef enum
{
    KEY_PBA_STEP_0,
    KEY_PBA_STEP_1,
    KEY_PBA_STEP_2,
    KEY_PBA_STEP_3,
    KEY_PBA_STEP_4,
    KEY_PBA_STEP_5,
    KEY_PBA_STEP_6,
    KEY_PBA_STEP_7,
    KEY_PBA_STEP_8,
    KEY_PBA_STEP_9,
    KEY_PBA_STEP_10,
    KEY_PBA_STEP_11,
    KEY_PBA_STEP_12,
    KEY_PBA_STEP_13,

} KeyPBAStepId_T;


void InitKeyPBA ( void );
void SetKeyPBAStep ( U8 mu8Val);
U8 GetKeyPBAStep ( void );

#endif
