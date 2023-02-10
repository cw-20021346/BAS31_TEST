
#include "hw.h"
#include "prj_type.h"
#include "process_key.h"


#include "key.h"
#include "Buzzer.h"
#include "SteppingMotor.h"
#include "FactoryMode.h"
#include "Setting.h"
#include "process_display.h"
#include "key_common_handler.h"
#include "key_handler.h"
#include "Aging.h"
#include "WaterTank.h"
#include "Waterlevel.h"
#include "Errorcheck.h"

//#if CONFIG_WATERLESS
//#include "Waterless.h"
//#endif

U8 gu8PopKeyStatePass = FALSE;
U8 gu8LongKeyPower = FALSE;

static U8 IsValidPopKeyFlag ( U8 mu8PopKeyPass, U8 mu8LongKeyPass )
{
    /* State default 설정 후(SET) pop하면 무시 */
    if ( mu8PopKeyPass == TRUE )
    {
        gu8PopKeyStatePass = FALSE;
        return FALSE;
    }


    /* 기능 동작중 전원키 입력시(SET) pop하거나 push하면 flag Clear */
    if ( mu8LongKeyPass == TRUE )
    {
        gu8LongKeyPower = FALSE;
        return FALSE;
    }

    return TRUE;

}


static U8 IsValidKeyCondition(U32 mu32Key)
{
    U32 mu32KeyMask = K_NONE;
    U8 mu8IsValidPopKeyResult;

    mu32KeyMask = GetKeyVal();

    /* Motor Initial || Factory Drain || initial 물채우기 */
    if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE
        || GetFactoryStart() == TRUE 
        || GetWaterLevelInitialStart() == TRUE 
//        || GetWaterLessInspection1Start() == TRUE
//        || GetWaterLessInspection2Start() == TRUE
        )
    {
        BUZZER_STEP( BUZZER_WARNING );
        return FALSE;
    }

    /* auto drain 상태에서 key 입력시 auto drain stop */
    if ( GetAutoDrainStart() == TRUE )
    {
        StopNozzleClean();
        BUZZER_STEP( BUZZER_SELECT );

        if ( mu32KeyMask == K_STATE )
        {
            gu8PopKeyStatePass = TRUE;
        }

        return FALSE;
    }
    
        mu8IsValidPopKeyResult = IsValidPopKeyFlag ( gu8PopKeyStatePass, gu8LongKeyPower );
        
        return mu8IsValidPopKeyResult;
        
}
    
static U8 IsValidPopKeyCondition(U32 mu32Key)
{
    U32 mu32KeyMask = K_NONE;
    U8 mu8IsValidPopKeyResult;

    mu32KeyMask = GetKeyVal();

    mu8IsValidPopKeyResult = IsValidPopKeyFlag ( gu8PopKeyStatePass, gu8LongKeyPower );
    
    return mu8IsValidPopKeyResult;
        

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
                    if( IsValidPopKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
                    SetWaterSupplyErrorReleaseKey ( TRUE ); 
                    mu8Sound = (pList+i)->actionEvent_4();

                    
                    /*2019-05-16 전원 OFF 키 pop시 buzzer skip 추가 */
                    if ( GetBuzzerSkip() == TRUE )
                    {
                        SetBuzzerSkip( FALSE );
                    }
                    else
                    {
                        BUZZER_STEP( mu8Sound );
                    }
                    
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
                    
                    SetWaterSupplyErrorReleaseKey ( TRUE );
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
                    if( IsValidKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
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
                    if( IsValidKeyCondition( mu32KeyVal ) == FALSE )
                    {
                        break;
                    }
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


    /* 초기 display 동작 || 메멘토 */
    /* Key 입력 불가 */
    if( GetDisplayInit() == FALSE
        || GetMementoDisp() == TRUE )
    {
        return ;
    }

    if ( GetSettingTime() != 0 )
    {
        //SpecialSetting Mode
        pKeyEventList = (KeyEventList_T *)GetKeySettingEventList();
        mu8KeyEventSize = GetKeySettingEventListSize();
    }
    else
    {
        if ( GetAgingStart() == TRUE )
        {
            //Aging Mode
            pKeyEventList = (KeyEventList_T *)GetKeyAgingEventList();
            mu8KeyEventSize = GetKeyAgingEventListSize();
        }
        else if ( GetFactoryTest_Remote() == TRUE )
        {
            //Factory Auto Test Mode
            pKeyEventList = (KeyEventList_T *)GetKeyAutoTestEventList();
            mu8KeyEventSize = GetKeyAutoTestEventListSize();
        }
        else if ( GetFactoryTest() == TRUE )
        {
            //Factory Manual Test Mode
            pKeyEventList = (KeyEventList_T *)GetKeyTestEventList();
            mu8KeyEventSize = GetKeyTestEventListSize();
        }
        else
        {
            // NORMAL
            pKeyEventList = (KeyEventList_T *)GetKeyEventList();
            mu8KeyEventSize = GetKeyEventListSize();
        }
    }


#if 0
    // EOL
    pKeyEventList = (KeyEventList_T *)GetEolKeyEventList();
    mu8KeyEventSize = GetEolKeyEventListSize();

    // TEST
    pKeyEventList = (KeyEventList_T *)GetTestKeyEventList();
    mu8KeyEventSize = GetTestKeyEventListSize();
#endif

    KeyEventHandler( pKeyEventList, mu8KeyEventSize );
}
