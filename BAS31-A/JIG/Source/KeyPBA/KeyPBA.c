#include "KeyPBA.h"

typedef struct _keypba_
{
    U8 Step;
    
} KeyPBA_T;

KeyPBA_T keypba;

void InitKeyPBA ( void )
{
    keypba.Step = KEY_PBA_STEP_0;
}

void SetKeyPBAStep ( U8 mu8Val)
{
    keypba.Step = mu8Val;
}

U8 GetKeyPBAStep ( void )
{
    return keypba.Step;
}
