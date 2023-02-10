
#include "hw.h"
#include "prj_type.h"
#include "process_key.h"


#include "key.h"
#include "Buzzer.h"
#include "process_display.h"
#include "key_common_handler.h"
#include "key_handler.h"


U8 gu8PopKeyStatePass = FALSE;
U8 gu8LongKeyPower = FALSE;


static U8 IsValidKeyCondition(U32 mu32Key)
{
    U16 mu16KeyMask = K_NONE;

    return TRUE;
}


static void KeyEventHandler(KeyEventList_T *pList, U8 mu8Size )
{
    U8   i;
    U32  mu32KeyVal;
    U32  mu32KeyPopVal;
    U8   mu8Sound;

    mu32KeyVal    = GetKeyVal();
    mu32KeyPopVal = GetKeyPopVal();
    
    /* Reset Unused Saving - HOT WATER */
    if( mu32KeyVal != K_NONE || mu32KeyPopVal != K_NONE )
    {
        //ResetUnusedSavingTime();
        //WakeUpSavingMode();
    }

    /* Check Pop Key */
    if( mu32KeyVal == K_NONE 
            && IsSetKeyEventFlag( KEY_EVENT_POP ) == TRUE )
    {
        for( i = 0; i < mu8Size ; i++ )
        {
            if( (pList+i)->Key == mu32KeyPopVal )
            {
                ClearKeyEventFlag( KEY_EVENT_POP );

                if( (pList+i)->actionEvent_4 != NULL )
                {
                    if( IsValidKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
                    
                    mu8Sound = (pList+i)->actionEvent_4();
                    BUZZER_STEP( mu8Sound );
                    return ;
                }
            }
        }
    }
    
    /* Check Pusehd Key */
    for( i = 0; i < mu8Size ; i++ )
    {
        if( (pList+i)->Key == mu32KeyVal )
        {
            if( IsSetKeyEventFlag( KEY_EVENT_PUSHED_SHORT_ON ) == TRUE )
            {
                ClearKeyEventFlag( KEY_EVENT_PUSHED_SHORT_ON );

                if( (pList+i)->actionEvent_1 != NULL )
                {
                    if( IsValidKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
                    
                    mu8Sound = (pList+i)->actionEvent_1();
                    BUZZER_STEP( mu8Sound );
                }
                break;
            }
            else if( IsSetKeyEventFlag( KEY_EVENT_PUSHED_SHORT_OFF ) == TRUE )
            {
                ClearKeyEventFlag( KEY_EVENT_PUSHED_SHORT_OFF );

                if( (pList+i)->actionEvent_5 != NULL )
                {
                    if( IsValidKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
                    mu8Sound = (pList+i)->actionEvent_5();
                    BUZZER_STEP( mu8Sound );
                }
                break;
            }
            else if( IsSetKeyEventFlag( KEY_EVENT_PUSHED_LONG_5S ) == TRUE )
            {
                ClearKeyEventFlag( KEY_EVENT_PUSHED_LONG_5S );

                if( (pList+i)->actionEvent_3 != NULL )
                {
                    mu8Sound = (pList+i)->actionEvent_3();
                    BUZZER_STEP( mu8Sound );
                }
                break;
            }
            else if( IsSetKeyEventFlag( KEY_EVENT_PUSHED_LONG_2S ) == TRUE )
            {
                ClearKeyEventFlag( KEY_EVENT_PUSHED_LONG_2S );

                if( (pList+i)->actionEvent_2 != NULL )
                {
                    mu8Sound = (pList+i)->actionEvent_2();
                    BUZZER_STEP( mu8Sound );
                }
                break;
            }
            else{}
        }
    }
}



void Evt_1msec_ProcessKeyEventHandler( void )
{
    KeyEventList_T  *pKeyEventList = NULL;
    U8 mu8KeyEventSize = 0;


    /* 초기 display 동작 || 메멘토 || 초기 급수 */
    /* Key 입력 불가 */
    if( GetDisplayInit() == FALSE
        || GetMementoDisp() == TRUE )
    {
        return ;
    }



    pKeyEventList = (KeyEventList_T *)GetKeyEventList();
    mu8KeyEventSize = GetKeyEventListSize();

    KeyEventHandler( pKeyEventList, mu8KeyEventSize );
}
