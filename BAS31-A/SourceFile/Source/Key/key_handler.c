#include "hw.h"

#include "key_handler.h"
#include "key_common_handler.h"

#include "Buzzer.h"
#include "eeprom.h"
#include "error.h"
#include "hal_key.h"

#include "Mode.h"
#include "SteppingMotor.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "Dry.h"
#include "SeatHeater.h"
#include "Aging.h"
#include "PowerSave.h"
#include "Process_Display.h"

#include "Sterilize.h"
#include "ProcessError.h"
#include "Valve.h"
#include "Seat.h"
#include "Setting.h"
#include "FactoryMode.h"
#include "Nozzle.h"

#include "Power.h"
#include "Powersave.h"

#include "Hotwater.h"
#include "Waterlevel.h"
#include "WaterTank.h"
#include "Remote.h"
#include "Autoflushing.h"

U8 gu8BuzzerSkip = FALSE;           /* 전원 OFF버튼 Pop Key 입력시 부저음 Skip */
extern U8 gu8PopKeyStatePass;
extern U8 gu8LongKeyPower;


/*Main Function*/
static U8 SelPower ( void );
static U8 BuzzerSkip ( void );
static U8 OnPower ( void );
static U8 SelStop ( void );

static U8 SelClean( void );
static U8 SelBidet( void );
static U8 SelDry( void );
static U8 SelServiceDry ( void );
static U8 SelState ( void );
static U8 PowerOffState ( void );

static U8 SelStateDefault ( void );


static U8 SelSter ( void );
static U8 SelNozzleClean ( void );

static U8 SelSeatTemp( void );
static U8 SelWaterTemp( void );

/*Press*/
static U8 SelPressBack( void );
static U8 SelPressFront( void );

/*Nozzle*/
static U8 SelNozzleBack( void );
static U8 SelNozzleFront( void );

/* not used */
#if 0
/*Move*/
static U8 SelMoveStart( void );

static U8 SelMood ( void );

static U8 SelPowerSave ( void );
static U8 OnPowerSave ( void );
#endif

static U8 PowerOffStop( void );
static U8 PowerOffPower( void );
static U8 PowerOffNozzleClean( void );




/*Multy Key*/
static U8 OnSetting( void );
static U8 RemoteParing ( void );

static U8 SelAgingMode( void );
static U8 SelFactoryDrain( void );



void SetBuzzerSkip ( U8 mu8buzzerskip )
{
    gu8BuzzerSkip = mu8buzzerskip;
}

U8 GetBuzzerSkip ( void )
{
    return gu8BuzzerSkip;
}



KeyEventList_T KeyEventList[] =
{
    /* KEY,       Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec, Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SelClean,       NULL,       NULL,               NULL,     NULL        	},
    { K_BIDET,          SelBidet,       NULL,       NULL,               NULL,     NULL        	},
    { K_STATE,          NULL,           NULL,       SelStateDefault,    SelState, PowerOffState },
    { K_STOP,           SelStop,        NULL,       NULL,               NULL,     PowerOffStop	},
                                                       
    { K_PRESS_UP,       SelPressFront,  NULL,       NULL,               NULL,     NULL        	},
    { K_PRESS_DOWN,     SelPressBack,   NULL,       NULL,               NULL,     NULL        	},
    { K_NOZZLE_UP,      SelNozzleFront, NULL,       NULL,               NULL,     NULL        	},
    { K_NOZZLE_DOWN,    SelNozzleBack,  NULL,       NULL,               NULL,     NULL        	},

    { K_DRY,            SelDry,         NULL,       SelServiceDry,      NULL,     NULL        	},                                                        
    { K_STER,           SelSter,        NULL,       NULL,               NULL,     NULL            },
        
    { K_WATER_TEMP,     SelWaterTemp,   NULL,       NULL,               NULL,     NULL        	},
    { K_SEAT_TEMP,      SelSeatTemp,    NULL,       NULL,               NULL,     NULL        	},
                                                                         
    { K_NOZZLE_CLEAN,   SelNozzleClean, NULL,       NULL,               NULL,     PowerOffNozzleClean	},
    { K_POWER,          SelPower,       OnPower,    NULL,               BuzzerSkip,     PowerOffPower   },


    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,         NULL,     NULL        	},
//    { K_MOOD,           SelMood,        NULL,           NULL,         NULL,     NULL        	},
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,         NULL,     NULL        	},

    /* Side Key */
    { K_SIDE_1,         SelClean,       NULL,           NULL,         NULL,     NULL        	},
    { K_SIDE_2,         SelBidet,       NULL,           NULL,         NULL,     NULL        	},
    { K_SIDE_3,         SelStop,        NULL,           NULL,         NULL,     NULL        	},

    /* MULTI KEY */                                     
    { K_SETTING,        NULL,           OnSetting,      NULL,         NULL,     NULL        	},
    { K_REMOTE_PARING,  NULL,           RemoteParing,   NULL,         NULL,     NULL        	},
    { K_AGING,          NULL,           SelAgingMode,   NULL,         NULL,     NULL        	},
    { K_FACTORY_MODE,   NULL,           SelFactoryDrain,NULL,         NULL,     NULL        	}
};


void* GetKeyEventList(void)
{
    return KeyEventList;
}

U8 GetKeyEventListSize(void)
{
    return ( sizeof(KeyEventList) / sizeof(KeyEventList_T) );
}


static U8 SelState ( void )
{
    U8  mu8Sound;
    
    /* Push의 경우 Power On/Off 구분하지만, pop은 구분 하지 않아서 조건 추가*/
    if ( IsErrorTypeStop() == TRUE 
        || GetPowerOnOff() == FALSE )
    {
        return BUZZER_NONE;

    }
    EcoPowerSaveMode_Stop();


    if ( GetModeState() == STATE_BASIC )
    {
        SetModeState ( STATE_ACTIVE );
    }
    else if ( GetModeState() == STATE_ACTIVE )
    {
        SetModeState ( STATE_CARE );
    }
    else //if ( GetModeState() == STATE_CARE )
    {
        SetModeState ( STATE_BASIC );
    }

    /* 2018-04-02 BX 한서리님 기능 동작 중 Mode 버튼 입력시 노즐 신장된 상태에서 Pre Wash부터 시작 */
    if ( ( GetModeSel() == TYPE_CLEAN 
        && IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE ) 
        || ( GetModeSel() == TYPE_BIDET 
        && IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) )
    {
        SetWaterPressUserPress ( FALSE ); 

        /* Pre Wash ~ Rinse Wash에서 Mode 입력시 Pre Wash부터 시작 */
        if ( GetMainStep() > STEP_MAIN_START_ING 
            && GetMainStep() < STEP_MAIN_DONE )
        {
            SetMainStep ( STEP_MAIN_PRE_SETTING );
        }
        else
        {
        }
    }

    mu8Sound = BUZZER_SELECT;
    
    
    return mu8Sound;

}

static U8 SelStateDefault ( void )
{
    U8 mu8Sound;
    U8 mu8ModeState;

    mu8ModeState = GetModeState();
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;

    }
    EcoPowerSaveMode_Stop();

    /* 2018-07-30 김종민QM님 기능 동작 중 Default 수정 가능하게 할것 */
    SetModeStateDefault ( mu8ModeState );
    SaveEepromId( EEP_ID_STATE_DEFAULT );

    mu8Sound = BUZZER_SET_ON;

#if 0    
    /* State 변경은 대기 Mode에서만 가능하다 */
    if ( GetModeSel() == TYPE_WAIT 
        && GetSterAll() == FALSE 
        && GetNozzleClean() == FALSE )
    {
        SetModeStateDefault ( mu8ModeState );
        SaveEepromId( EEP_ID_STATE_DEFAULT );

        mu8Sound = BUZZER_SET_ON;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
#endif

    gu8PopKeyStatePass = TRUE;

    return mu8Sound;
}


static U8 SelClean( void )
{

    U8 mu8Sound = 0;

    /* Power On/Off는 진입 전 구별하지만, Mode State 변경시 간헐적으로 눌림 방지 */
    if ( IsErrorTypeStop() == TRUE 
        || GetPowerOnOff() == FALSE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( ( GetSeatSetting() == TRUE 
            && GetSeatLevel() == FALSE )
        || GetWaterLevel() == FALSE 
        || GetWaterLevelAddWaterTime() != 0 )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {

        //2015-01-16 통합살균 중 세정 버튼 입력시 밸브 OFF
        //2015-01-16 통합살균 중 세정 버튼 입력시 밸브 OFF
        if( GetSterAll() == TRUE )
        {
            Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
            EW_Stop();                //살균 중이었으면 멈추고
        }
            
        if ( GetNozzleClean() == TRUE )
        {
            StopNozzleClean();
        }

        if ( GetModeState() == STATE_ACTIVE )
        {
            SetModeFlag ( MODE_FLAG_CLEAN_ACTIVE );
        }
        else if ( GetModeState() == STATE_CARE )
        {
            SetModeFlag ( MODE_FLAG_CLEAN_CARE );
        }
        else //if ( GetModeState() == STATE_NORMAL )
        {
            SetModeFlag ( MODE_FLAG_CLEAN_NORMAL );
        }
     
      mu8Sound = BUZZER_SELECT;
    }   
    return mu8Sound;
}



static U8 SelBidet( void )
{
    U8 mu8Sound;
    
    if ( IsErrorTypeStop() == TRUE)
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    
    if ( ( GetSeatSetting() == TRUE 
            && GetSeatLevel() == FALSE )
        || GetWaterLevel() == FALSE 
        || GetWaterLevelAddWaterTime() != 0 )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        //2015-01-16 통합살균 중 비데 버튼 입력시 밸브 OFF
        if( GetSterAll() == TRUE )
        {
            Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
            EW_Stop();               
        }

        if ( GetNozzleClean() == TRUE )
        {
            StopNozzleClean();
        }

        if ( GetModeState() == STATE_ACTIVE )
        {
            SetModeFlag ( MODE_FLAG_BIDET_ACTIVE );
        }
        else if ( GetModeState() == STATE_CARE )
        {
            SetModeFlag ( MODE_FLAG_BIDET_CARE );
        }
        else //if ( GetModeState() == STATE_NORMAL )
        {
            SetModeFlag ( MODE_FLAG_BIDET_NORMAL );
        }

        mu8Sound = BUZZER_SELECT;
    }
    return mu8Sound;
}


static U8 BuzzerSkip ( void )
{
    SetBuzzerSkip ( TRUE );
    gu8PopKeyStatePass = FALSE;

    return BUZZER_NONE;
}



static U8 Stop ( void );

static U8 OnPower ( void )
{
#if 0       // IsValidKeyCondition()에서 Clear
    if ( gu8LongKeyPower == TRUE )
    {
        gu8LongKeyPower = FALSE;
        return BUZZER_NONE;
    }
#endif

    EcoPowerSaveMode_Stop();
    SetPowerOnOff ( FALSE );
    SetSterReservation ( FALSE );
    return BUZZER_POWER_OFF;
}



static U8 SelPower ( void )
{
    //2014-12-04 이승용 선임. 기능 동작 중 전원 Long 키 입력시 전원 OFF 방지
    if ( GetModeSel() != TYPE_WAIT 
        || GetNozzleClean() == TRUE 
        || GetSterAll() == TRUE )
    {
        gu8LongKeyPower = TRUE;
    }

    
    
    EcoPowerSaveMode_Stop();
    
    return Stop();
} 


static U8 SelStop ( void )
{
    U8 mu8Sound;

    
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }

    EcoPowerSaveMode_Stop();

    return Stop();
}

static U8 Stop ( void ) 
{
	U8 mu8Sound;

    SetSterReservation ( FALSE );
    
    //2015-01-16 통합살균 중 건조 버튼 입력시 밸브 OFF
    if(GetSterAll() == TRUE) 
    {
        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
        EW_Stop();                //살균 중이었으면 멈추고
        mu8Sound = BUZZER_SELECT;
    }
    else if ( GetNozzleClean() == TRUE )
    {
        StopNozzleClean();
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        SetModeFlag ( MODE_FLAG_STOP );
        mu8Sound = BUZZER_SELECT;
    }
    
    return mu8Sound;
}




static U8 SelDry( void )
{
    U8 mu8Sound;
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    
    if(GetSeatSetting() == TRUE 
        && GetSeatLevel()==FALSE )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {                           
        //2015-01-16 통합살균 중 건조 버튼 입력시 밸브 OFF
        if( GetSterAll() == TRUE )
        {   
            Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
            EW_Stop();      //살균 중이었으면 멈추고          
        }

        /* 2018-07-24 산제성님.body Ster 중 건조버튼 입력시 body Ster Clear 안되서 EW ON 되는 동작 방지 */
        if ( GetSterBody() == TRUE )
        {
            EW_Stop();      //살균 중이었으면 멈추고          
        }
        
        if ( GetNozzleClean() == TRUE )
        {
            StopNozzleClean();
        } 

        SetModeFlag ( MODE_FLAG_DRY );
        mu8Sound = BUZZER_SELECT;
    }
    return mu8Sound;
}

static U8 SelServiceDry ( void )
{
    U8 mu8Sound;
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
  
    //2015-01-16 통합살균 중 건조 버튼 입력시 밸브 OFF
    if( GetSterAll() == TRUE )
    {
    	Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
    	Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
    	EW_Stop();                //살균 중이었으면 멈추고
    }
    
    if ( GetNozzleClean() == TRUE )
    {
        StopNozzleClean();
    } 

    SetDryService(TRUE);
    SetModeFlag ( MODE_FLAG_DRY );

    mu8Sound = BUZZER_SELECT;

    return mu8Sound;

}

static U8 SelSter ( void )
{
    U8 mu8Sound;

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( GetWaterLevel() == FALSE )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        /* All Ster ing */    
        if( GetSterAll() == TRUE )
        {
            EW_Stop();
            Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
            mu8Sound = BUZZER_SELECT;
        }
        else
        {   
            /* 살균 Reservation */
            if ( GetSeatLevel() == TRUE
                || GetModeSel() == TYPE_CLEAN 
                || GetModeSel() == TYPE_BIDET 
                || ( GetModeSel() == TYPE_DRY 
                    && GetDryService() == FALSE ) )
            {
                if ( GetSterReservation() == FALSE )
                {
                    SetSterReservation ( TRUE );
                    mu8Sound = BUZZER_SET_ON;
                }
                else
                {
                    SetSterReservation ( FALSE );
                    mu8Sound = BUZZER_SET_OFF;
                }
            }
            /* 살균 Start */
            else
            {
                SetModeFlag ( MODE_FLAG_STOP );
                SterAllStart();
                mu8Sound = BUZZER_SELECT;
            }
        }
    }
    return mu8Sound;
}







static U8 SelNozzleClean ( void )
{
    U8 mu8Sound;
    
    if (IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( GetModeSel() == TYPE_WAIT )
    {
        if(GetSterAll() == TRUE)
        {
            mu8Sound = BUZZER_WARNING;
        }
        else
        {
            if ( GetNozzleClean() == TRUE )
            {
                StopNozzleClean();
            }
            else
            {
                SetNozzleClean ( TRUE );
            }
            mu8Sound = BUZZER_SELECT;
        }
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    return mu8Sound;
}
static U8 SelSeatTemp( void )
{

    U8 mu8SeatLevel;
    U8 mu8Sound;

    

    mu8SeatLevel = GetSeatHeaterLevel();

	if( IsErrorTypeStop() == TRUE )
	{
	    return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( mu8SeatLevel <= 2 )
    {
        mu8SeatLevel++;
    }
    else
    {
        mu8SeatLevel = 0;
    }
    
    SetSeatHeaterLevel ( mu8SeatLevel );
    mu8Sound = BUZZER_SELECT;
    
    return mu8Sound;
}



static U8 SelWaterTemp( void )
{
    U8 mu8DryLevel;
    U8 mu8WaterLevel;
    U8 mu8Sound;
    
    mu8DryLevel = GetDryLevel();
    mu8WaterLevel = GetHotLevel();

    if( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
        
    EcoPowerSaveMode_Stop();

    //건조 모드 일때
    if ( GetModeSel() == TYPE_DRY )
    {
        if( GetDryService() == FALSE )     //서비스건조모드에서는 건조온도 변경 불가
        {
            if ( mu8DryLevel <= 2 )
            {
                mu8DryLevel++;
            }
            else
            {
                mu8DryLevel = 0;
            }
            SetDryLevel(mu8DryLevel);
            mu8Sound = BUZZER_SELECT;
        }
        else
        {
            mu8Sound = BUZZER_WARNING;
        }
    }
    else
    {
        if ( mu8WaterLevel <= 2 )
        {
            mu8WaterLevel++;
        }
        else
        {
            mu8WaterLevel = 0;
        }
        
        SetHotLevel ( mu8WaterLevel );
        mu8Sound = BUZZER_SELECT;
    }    

    return mu8Sound;
}















static U8 SelPressBack (void)
{
    U8 mu8Sound;
    U8 mu8Level;
    
    mu8Level = GetWaterPressLevel();

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( ( mu8Level > PRESS_MINIUM ) && ( mu8Level <= PRESS_MAXIUM ) )
    {
        mu8Level--;
        SetWaterPressLevel( mu8Level );
        SetWaterPressLevelTemp ( mu8Level );

        //SetFlowPressCompensationCount(0);   //모터 교정 횟수
        SetWaterPressDelay( WATER_PRESS_CHANGE_DELAY );

        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }

    /* 세정/여성세정 중 수압 버튼 입력시 i-wave 수압이 아닌 사용자 설정 수압으로 변경 */
    if ( GetModeSel() == TYPE_CLEAN
        || GetModeSel() == TYPE_BIDET )
    {
        SetWaterPressUserPress ( TRUE ); 
    }
        
    return mu8Sound; 
}


static U8 SelPressFront(void)
{
    U8 mu8Sound;
    U8 mu8Level;

    mu8Level = GetWaterPressLevel();

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }


    EcoPowerSaveMode_Stop();
   
    //SetNozzleDisplay( 0 );        /*노즐 디스플레이 OFF*/

    if ( ( mu8Level >= PRESS_MINIUM ) && ( mu8Level < PRESS_MAXIUM ) )
    {
        mu8Level++;
        SetWaterPressLevel( mu8Level );
        SetWaterPressLevelTemp ( mu8Level );
        
        //SetFlowPressCompensationCount(0);   //모터 교정 횟수
        SetWaterPressDelay( WATER_PRESS_CHANGE_DELAY );
        
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }

    /* 세정/여성세정 중 수압 버튼 입력시 i-wave 수압이 아닌 사용자 설정 수압으로 변경 */
    if ( GetModeSel() == TYPE_CLEAN
        || GetModeSel() == TYPE_BIDET )
    {
        SetWaterPressUserPress ( TRUE ); 
    }

    return mu8Sound;
}

#define NOZZLE_DISPLAY      300     /*10msec * 300 = 3sec */
#define NOZZLE_DELAY        15      /*10msec * 15 = 150msec */

static U8 SelNozzleBack(void)
{
    U8 mu8Sound;
    U8 mu8Level;
    
    mu8Level = GetNozzleLevel();

    if ( IsErrorTypeStop() == TRUE )
	{
	    return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    /* 연속동작 방지 */
    if ( GetNozzleDelay() != 0 )
    {
        return BUZZER_NONE;
    }
    
    SetNozzleDelay ( NOZZLE_DELAY );
    //SetNozzleDisplay( NOZZLE_DISPLAY );

    if( ( mu8Level > NOZZLE_MINIUM ) && ( mu8Level <= NOZZLE_MAXIUM ) )
    {
        mu8Level--;
        SetNozzleLevel ( mu8Level );
        SetNozzleLevelTemp ( mu8Level );
        
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Level = NOZZLE_MINIUM;
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound; 
}

static U8 SelNozzleFront(void)
{
    U8 mu8Sound;
    U8 mu8Level;
    
    mu8Level = GetNozzleLevel();

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    /* 연속동작 방지 */
    if ( GetNozzleDelay() != 0 )
    {
        return BUZZER_NONE;
    }
    
    SetNozzleDelay ( NOZZLE_DELAY );
	//SetNozzleDisplay( NOZZLE_DISPLAY );

    if( ( mu8Level >= NOZZLE_MINIUM ) && ( mu8Level < NOZZLE_MAXIUM ) )
	{
	    mu8Level++;
        SetNozzleLevel ( mu8Level );
        SetNozzleLevelTemp ( mu8Level );

	    mu8Sound = BUZZER_SELECT;
	}
	else
    {
        mu8Level = NOZZLE_MAXIUM;
        mu8Sound = BUZZER_WARNING;
	}
    return mu8Sound;
}
/* not used */
#if 0
static U8 SelMoveStart(void)
{
    U8 mu8Sound; 
    U8 mu8Move;
    
    mu8Move = GetMoveStart();

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }

    EcoPowerSaveMode_Stop();

    if ( ( ( GetModeSel() == TYPE_CLEAN )
            && ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE ) )
        || ( ( GetModeSel() == TYPE_BIDET )
            && ( IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) ) )
    {
        if ( mu8Move == TRUE )
        {
            mu8Move = FALSE;
        }
        else
        {
            mu8Move = TRUE;
        }
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Move = FALSE;
        mu8Sound = BUZZER_WARNING;
    }
    SetMoveStart ( mu8Move );
    return mu8Sound;
}

static U8 SelMood ( void )
{
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    ToggleSettingMood();
    return BUZZER_SELECT;
}


static U8 ChangePowerSaveMode ( void );
static U8 SelPowerSave ( void )
{
    U8 mu8Sound;
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop(); 
    

    if ( GetFactoryTest() == FALSE )
    {
        mu8Sound = BUZZER_SELECT;
    }
    /*공장테스트모드에서 바로 진입*/
    else
    {
        mu8Sound = ChangePowerSaveMode();
    }

    return mu8Sound;
}

static U8 OnPowerSave ( void )
{
    U8 mu8Sound;
        
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();  

    if ( GetFactoryTest() == FALSE )
    {
        mu8Sound = ChangePowerSaveMode();
    }
    /*공장테스트모드에서 바로 진입*/
    else
    {
        mu8Sound = BUZZER_NONE;        
    }

    return mu8Sound;
}
        
static U8 ChangePowerSaveMode ( void )
{
    U8 mu8Sound;
    
    //2014-10-13 구남진 선임. 그린플러그 기능동작 중에서 설정 가능하도록 수정요청
    if ( GetPowerSaveMode() == POWER_SAVE_NONE)
    {
        SetPowerSaveMode ( POWER_SAVE_SET );
        mu8Sound = BUZZER_GREENPLUG_ON;
    }
    else //if ( GetPowerSaveMode() == POWER_SAVE_SET )
    {
        SetPowerSaveMode ( POWER_SAVE_NONE );
        mu8Sound = BUZZER_GREENPLUG_OFF;
    }

    return mu8Sound;

}
#endif
static void NextStepNozzleClean ( void );
static U8 PowerOffNozzleClean( void )
{
    U8 mu8Sound;
    if ( GetNozzleClean() == TRUE )
    {
        NextStepNozzleClean();
        mu8Sound = BUZZER_NOZZLE_CLEAN;
    }
    else
    {
        SetNozzleClean ( TRUE );
        mu8Sound = BUZZER_NOZZLE_CLEAN;
    }
    return mu8Sound;
}

static U8 PowerOffStop( void )
{
    U8 mu8Sound;
    if ( GetNozzleClean() == TRUE )
    {
        NextStepNozzleClean();
        mu8Sound = BUZZER_NOZZLE_CLEAN;
    }    
    else
    {
        mu8Sound = BUZZER_NONE;
    }
    return mu8Sound;
}


static U8 PowerOffPower( void )
{
    U8 mu8Sound;

    /* 2018-08-31 이재섭님. 전원 OFF 상태에서 long키 입력시 다시 전원 OFF 되는 오류 방지 */
    gu8LongKeyPower = TRUE;
    
    if ( GetNozzleClean() == TRUE )
    {
        NextStepNozzleClean();
        mu8Sound = BUZZER_NOZZLE_CLEAN;
    }
    else
    {
        SetPowerOnOff( TRUE );
        mu8Sound = BUZZER_POWER_ON;
    }
    return mu8Sound;
}

static void NextStepNozzleClean ( void )
{
    if ( GetNozzleCleanStep() >= CHECK_CLEAN_NOZZLE 
        && GetNozzleCleanStep() <= END_CLEAN_NOZZLE )
    {
        SetNozzleCleanStep ( END_CLEAN_NOZZLE );
    }
    else if ( GetNozzleCleanStep() >= CHECK_BIDET_NOZZLE 
        && GetNozzleCleanStep() <= END_BIDET_NOZZLE )
    {
        SetNozzleCleanStep ( END_BIDET_NOZZLE );
    }
}

static U8 PowerOffState ( void )
{
    gu8PopKeyStatePass = TRUE;

    return BUZZER_NONE;
}

static U8 OnSetting ( void )
{
    U8 mu8Sound;

    if( IsErrorTypeStop() == TRUE )
	{
	    return BUZZER_NONE;
    }    

    if ( GetModeSel() == TYPE_WAIT
        && GetSterAll() == FALSE 
        && GetNozzleClean() == FALSE )
    {
        SetSettingTime(SETTING_TIME);
        EW_Stop();
        mu8Sound = BUZZER_SET_ON;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }

    return mu8Sound;
}

/* F_Remocon_Pairing_Mode == SET, Key 입력 범위 설정할 것 */
static U8 RemoteParing ( void )
{
    U8 mu8Sound;

    if ( GetRemoteParingStart() == TRUE )
    {
        SetRemoteParingStart ( FALSE );
    }
    else
    {
        SetRemoteParingStart ( TRUE );
    }

    mu8Sound = BUZZER_SET_ON;
    
    return mu8Sound;
}

static U8 SelFactoryDrain(void)
{
    U8 mu8Sound;    

    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }    

    //2014-10-21 구남진 선임. 후세척 중 잔수빼기 진입 가능
    if( GetSterAll() == FALSE 
        && GetNozzleClean() == FALSE
        && GetAgingStart() == FALSE
    /* 2018-06-15 Like DPP 중 수위 미감지로 인한 진입 불가 수정 */
//        && GetWaterLevel() == TRUE 
//        && GetWaterLevelAddWaterTime() == 0 
//        && GetWaterLevelInitialStart() == FALSE
        && GetFactoryStart() == FALSE )
    {
        SetModeFlag ( MODE_FLAG_STOP );
        EW_Stop();
            
        SetFactoryReady ( TRUE );
        mu8Sound = BUZZER_SET_ON;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;
}

static U8 SelAgingMode ( void )
{
    U8 mu8Sound;   

    if ( GetModeSel() == TYPE_WAIT 
        && GetSterAll() == FALSE 
        && GetNozzleClean() == FALSE
        && GetSterAfter() == FALSE
        && IsErrorTypeStop() == FALSE 
        && GetAgingTime() != 0 )
    {
        SetAgingStart ( TRUE );
        SetModeState ( STATE_ACTIVE );
        mu8Sound = BUZZER_SET_ON;
    }
    else
    {
        mu8Sound = BUZZER_NONE;
    }

    return mu8Sound;
}




static U8 SettingMute(void);
static U8 SettingSeat (void );
static U8 SettingSave ( void );
static U8 SettingBody ( void );
static U8 SettingAutoDrain ( void );
static U8 SettingPowerSave ( void );
static U8 SelManualFlushing ( void );



KeyEventList_T KeySettingEventList[] =
{
    /* KEY,             Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec, Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SettingBody,        NULL,               NULL,         NULL,     NULL        	},
    { K_BIDET,          SettingSeat,        NULL,               NULL,         NULL,     NULL        	},
    { K_STATE,          NULL,               NULL,               NULL,         NULL,     NULL            },
    { K_STOP,           SettingMute,        NULL,               NULL,         NULL,     NULL	        },
                                                        
    { K_PRESS_UP,       NULL,               NULL,               NULL,         NULL,     NULL        	},
    { K_PRESS_DOWN,     NULL,               NULL,               NULL,         NULL,     NULL        	},
    { K_NOZZLE_UP,      NULL,               NULL,               NULL,         NULL,     NULL        	},
    { K_NOZZLE_DOWN,    NULL,               NULL,               NULL,         NULL,     NULL        	},
                                                        
    { K_DRY,            NULL,               NULL,               NULL,         NULL,     NULL        	},
    { K_STER,           NULL,               NULL,               NULL,         NULL,     NULL            },

    { K_WATER_TEMP,     NULL,               NULL,               NULL,         NULL,     NULL        	},
    { K_SEAT_TEMP,      NULL,               SelManualFlushing,  NULL,         NULL,     NULL        	},
                                                                          
    { K_NOZZLE_CLEAN,   SettingAutoDrain,   NULL,               NULL,         NULL,     NULL	        },
    { K_POWER,          SettingPowerSave,   NULL,               NULL,         NULL,     NULL   	        },

    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,         NULL,     NULL          },
//    { K_MOOD,           SelMood,        NULL,           NULL,         NULL,     NULL          },
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,         NULL,     NULL          },

    /* Side Key */
    { K_SIDE_1,         NULL,               NULL,               NULL,         NULL,     NULL            },
    { K_SIDE_2,         NULL,               NULL,               NULL,         NULL,     NULL            },
    { K_SIDE_3,         NULL,               NULL,               NULL,         NULL,     NULL            },
                                                        
    /* MULTI KEY */                                     
    { K_SETTING,        NULL,               NULL,           NULL,         NULL,     NULL        	},
    { K_REMOTE_PARING,  NULL,               NULL,           NULL,         NULL,     NULL        	},
    { K_AGING,          NULL,               NULL,           NULL,         NULL,     NULL        	},
    { K_FACTORY_MODE,   NULL,               NULL,           NULL,         NULL,     NULL        	}



};



void* GetKeySettingEventList(void)
{
    return KeySettingEventList;
}

U8 GetKeySettingEventListSize(void)
{
    return ( sizeof(KeySettingEventList) / sizeof(KeyEventList_T) );
}


static U8 SettingMute(void)
{
    ToggleBuzOnOff();
    return BUZZER_SELECT;
}  

static U8 SettingSeat (void )
{
    ToggleSeatSetting();
    return BUZZER_SELECT;
}


static U8 SettingSave ( void )
{
    ToggleSettingSaveMode();
    return BUZZER_SELECT;
}

static U8 SettingBody ( void )
{
    ToggleSterSettingBody();
    return BUZZER_SELECT;
}

static U8 SettingAutoDrain ( void )
{
    ToggleSettingAutoDrain();
    return BUZZER_SELECT;
}

static U8 SettingPowerSave ( void )
{
    ToggleSettingPowerSave();
    return BUZZER_SELECT;
}

static U8 SelManualFlushing ( void )
{
    U8 mu8Sound;
    
    if ( GetAutoFlushingStart() == FALSE
        && GetSeatLevel() == FALSE )
    {
        SetAutoFlushingManual ( TRUE );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }

    return mu8Sound;
}






static U8 SelAgingRepeat ( void );
static U8 SelAgingContinuous ( void );

static U8 SelAgingStop ( void );
static U8 SelAgingPower ( void );

static U8 SelMemento ( void );
static U8 DelMemento ( void );





KeyEventList_T KeyAgingEventList[] =
{
    /* KEY,             Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec,       Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SelAgingContinuous, SelMemento,     NULL,                   NULL,     NULL        	},
    { K_BIDET,          SelAgingRepeat,     NULL,           NULL,                   NULL,     NULL        	},
    { K_STATE,          NULL,               NULL,           NULL,                   NULL,     NULL        	},
    { K_STOP,           SelAgingStop,       NULL,           NULL,                   NULL,     NULL	        },
                                                        
    { K_PRESS_UP,       SelPressFront,      NULL,           NULL,                   NULL,     NULL        	},
    { K_PRESS_DOWN,     SelPressBack,       NULL,           NULL,                   NULL,     NULL        	},
    { K_NOZZLE_UP,      SelNozzleFront,     NULL,           NULL,                   NULL,     NULL        	},
    { K_NOZZLE_DOWN,    SelNozzleBack,      NULL,           NULL,                   NULL,     NULL        	},

    { K_DRY,            NULL,               NULL,           NULL,                   NULL,     NULL        	},
    { K_STER,           NULL,               NULL,           NULL,                   NULL,     NULL        	},
    
    { K_WATER_TEMP,     SelWaterTemp,       NULL,           NULL,                   NULL,     NULL        	},
    { K_SEAT_TEMP,      SelSeatTemp,        NULL,           NULL,                   NULL,     NULL        	},
                                                                          
    { K_NOZZLE_CLEAN,   DelMemento,         NULL,           NULL,                   NULL,     NULL	        },
    { K_POWER,          SelAgingStop,       NULL,           NULL,                   NULL,     NULL   	        },

    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,                   NULL,     NULL          },
//    { K_MOOD,           SelMood,        NULL,           NULL,                   NULL,     NULL          },
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,                   NULL,     NULL          },

    /* Side Key */
    { K_SIDE_1,         NULL,               NULL,           NULL,                   NULL,     NULL            },
    { K_SIDE_2,         NULL,               NULL,           NULL,                   NULL,     NULL            },
    { K_SIDE_3,         NULL,               NULL,           NULL,                   NULL,     NULL            },

    /* MULTI KEY */                                     
    { K_SETTING,        NULL,               NULL,           NULL,                   NULL,     NULL        	},
    { K_REMOTE_PARING,  NULL,               NULL,           NULL,                   NULL,     NULL            },
    { K_AGING,          NULL,               NULL,           NULL,                   NULL,     NULL        	},
    { K_FACTORY_MODE,   NULL,               NULL,           NULL,                   NULL,     NULL        	}



};

void* GetKeyAgingEventList(void)
{
    return KeyAgingEventList;
}

U8 GetKeyAgingEventListSize(void)
{
    return ( sizeof(KeyAgingEventList) / sizeof(KeyEventList_T) );
}

static U8 SelAgingRepeat ( void )
{
    U8 mu8Sound = 0;
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    
    if ( ( GetSeatSetting() == TRUE 
        && GetSeatLevel()==FALSE )
        || GetWaterLevel() == FALSE 
        || GetWaterLevelAddWaterTime() != 0 )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        SetAgingMode ( AGING_MODE_REPEAT );

        //세정 -> 비데 -> 건조 -> 세정 ...
        SetModeFlag ( MODE_FLAG_CLEAN_ACTIVE );
	    EW_Stop();

        mu8Sound = BUZZER_SELECT;
    }
    return mu8Sound;
}

static U8 SelAgingContinuous (void )
{
    U8 mu8Sound = 0;
    
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    
    if ( ( GetSeatSetting() == TRUE 
        && GetSeatLevel()==FALSE )
        || GetWaterLevel() == FALSE 
        || GetWaterLevelAddWaterTime() != 0 )
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        SetAgingMode ( AGING_MODE_CONTINUE );

        /* 액티브 세정 연속 동작 */
        SetModeFlag ( MODE_FLAG_CLEAN_ACTIVE );
        EW_Stop();

        mu8Sound = BUZZER_SELECT;
    }
    return mu8Sound;
}


static U8 SelAgingStop ( void )
{
    if ( IsErrorTypeStop() == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    if ( GetAgingMode() == AGING_MODE_REPEAT )
    {
        //에이징모드에서 세정이었으면 비데
        if ( GetModeSel() == TYPE_CLEAN )
        {
            SetModeFlag ( MODE_FLAG_BIDET_ACTIVE );
        }
        else if ( GetModeSel() == TYPE_BIDET )
        {
            SetModeFlag ( MODE_FLAG_DRY );
        }
        else if ( GetModeSel() == TYPE_DRY )
        {
            SetModeFlag ( MODE_FLAG_STOP );
        }
        else
        {
            SetAgingMode ( AGING_MODE_NONE );
            EW_Stop();
        }
    }

    if ( GetAgingMode() == AGING_MODE_CONTINUE )
    {
        SetAgingMode ( AGING_MODE_NONE );
        SetModeFlag ( MODE_FLAG_STOP );
    }

    return BUZZER_SELECT;
}







static U8 SelMemento ( void )
{
    if ( IsErrorTypeStop() == TRUE)
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();
    SetAgingStart ( FALSE );
    SetAgingMode ( AGING_MODE_NONE );
    
    SetMementoDisp(TRUE);

    TURN_OFF_WATER_HEATER();
#if 0
    TURN_OFF_WATER_HEATER_1();
    TURN_OFF_WATER_HEATER_2();
#endif
    TURN_OFF_SEAT_HEATER();

    TURN_OFF_AIR_PUMP();
       

    /*2016-04-02. 특수설정모드에서 memento 진입시 특수설정모드 해제*/
    SetSettingTime(0);

    if( GetSterAll() == TRUE )
    {
        EW_Stop();
        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );    
        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
    }

    SetModeFlag ( MODE_FLAG_STOP );
}

static U8 DelMemento (void)
{
    if ( GetAgingMode() != AGING_MODE_NONE )
    {
        return BUZZER_NONE;
    }
    ClearMementoError();

    return BUZZER_SELECT;
}

static U8 SelStateTest ( void );

KeyEventList_T KeyTestEventList[] =
{
    /* KEY,       Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec, Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SelClean,       NULL,       NULL,               NULL,           NULL        	},
    { K_BIDET,          SelBidet,       NULL,       NULL,               NULL,           NULL        	},
    { K_STATE,          NULL,           NULL,       SelStateDefault,    SelStateTest,   PowerOffState },
    { K_STOP,           SelStop,        NULL,       NULL,               NULL,           PowerOffStop	},
                                                       
    { K_PRESS_UP,       SelPressFront,  NULL,       NULL,               NULL,           NULL        	},
    { K_PRESS_DOWN,     SelPressBack,   NULL,       NULL,               NULL,           NULL        	},
    { K_NOZZLE_UP,      SelNozzleFront, NULL,       NULL,               NULL,           NULL        	},
    { K_NOZZLE_DOWN,    SelNozzleBack,  NULL,       NULL,               NULL,           NULL        	},

    { K_DRY,            SelDry,         NULL,       SelServiceDry,      NULL,           NULL        	},                                                        
    { K_STER,           SelSter,        NULL,       NULL,               NULL,           NULL            },
        
    { K_WATER_TEMP,     SelWaterTemp,   NULL,       NULL,               NULL,           NULL        	},
    { K_SEAT_TEMP,      SelSeatTemp,    NULL,       NULL,               NULL,           NULL        	},
                                                                         
    { K_NOZZLE_CLEAN,   SelNozzleClean, NULL,       NULL,               NULL,           PowerOffNozzleClean	},
    { K_POWER,          SelPower,       OnPower,    NULL,               NULL,           PowerOffPower   },

    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,         NULL,     NULL        	},
//    { K_MOOD,           SelMood,        NULL,           NULL,         NULL,     NULL        	},
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,         NULL,     NULL        	},

    /* Side Key */
    { K_SIDE_1,         SelClean,       NULL,           NULL,         NULL,             NULL        	},
    { K_SIDE_2,         SelBidet,       NULL,           NULL,         NULL,             NULL        	},
    { K_SIDE_3,         SelStop,        NULL,           NULL,         NULL,             NULL        	},

    /* MULTI KEY */                                     
    { K_SETTING,        NULL,           OnSetting,      NULL,         NULL,             NULL        	},
    { K_REMOTE_PARING,  NULL,           RemoteParing,   NULL,         NULL,             NULL        	},
    { K_AGING,          NULL,           SelAgingMode,   NULL,         NULL,             NULL        	},
    { K_FACTORY_MODE,   SelFactoryDrain,NULL,           NULL,         NULL,             NULL        	}
};

void* GetKeyTestEventList(void)
{
    return KeyTestEventList;
}

U8 GetKeyTestEventListSize(void)
{
    return ( sizeof(KeyTestEventList) / sizeof(KeyEventList_T) );
}

static U8 SelStateTest ( void )
{
    U8  mu8Sound;
    
    /* Push의 경우 Power On/Off 구분하지만, pop은 구분 하지 않아서 조건 추가*/
    if ( IsErrorTypeStop() == TRUE 
        || GetPowerOnOff() == FALSE )
    {
        return BUZZER_NONE;

    }
    EcoPowerSaveMode_Stop();


    if ( GetModeState() == STATE_BASIC )
    {
        SetModeState ( STATE_ACTIVE );
    }
    else if ( GetModeState() == STATE_ACTIVE )
    {
        SetModeState ( STATE_CARE );
    }
    else //if ( GetModeState() == STATE_CARE )
    {
        SetModeState ( STATE_BASIC );
    }

    /* 2018-04-02 BX 한서리님 기능 동작 중 Mode 버튼 입력시 노즐 신장된 상태에서 Pre Wash부터 시작 */
    if ( ( GetModeSel() == TYPE_CLEAN 
        && IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE ) 
        || ( GetModeSel() == TYPE_BIDET 
        && IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) )
    {
        SetWaterPressUserPress ( FALSE ); 

        /* Pre Wash ~ Rinse Wash에서 Mode 입력시 Main Wash부터 시작 */
        if ( GetMainStep() > STEP_MAIN_START_ING 
            && GetMainStep() < STEP_MAIN_DONE )
        {
            SetMainStep ( STEP_MAIN_MAIN_SETTING );
        }
        else
        {
        }
    }

    mu8Sound = BUZZER_SELECT;
    
    
    return mu8Sound;

}

static U8 SelAutoTestStart ( void );
static U8 SelAutoTestStop ( void );
static U8 SelWarnning ( void );
static U8 SelTestSter ( void );

KeyEventList_T KeyAutoTestEventList[] =
{
    /* KEY,       Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec, Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SelAutoTestStart,NULL,       NULL,               NULL,           NULL        	},
    { K_BIDET,          NULL,           NULL,       NULL,               NULL,           NULL        	},
    { K_STATE,          NULL,           NULL,       NULL,               NULL,           NULL            },
    { K_STOP,           SelAutoTestStop,NULL,       NULL,               NULL,           NULL	        },
                                                       
    { K_PRESS_UP,       NULL,           NULL,       NULL,               NULL,           NULL        	},
    { K_PRESS_DOWN,     NULL,           NULL,       NULL,               NULL,           NULL        	},
    { K_NOZZLE_UP,      NULL,           NULL,       NULL,               NULL,           NULL        	},
    { K_NOZZLE_DOWN,    NULL,           NULL,       NULL,               NULL,           NULL        	},

    { K_DRY,            NULL,           NULL,       NULL,               NULL,           NULL        	},                                                        
    { K_STER,           SelTestSter,    NULL,       NULL,               NULL,           NULL            },
        
    { K_WATER_TEMP,     NULL,           NULL,       NULL,               NULL,           NULL        	},
    { K_SEAT_TEMP,      SelSeatTemp,    NULL,       NULL,               NULL,           NULL        	},
                                                                         
    { K_NOZZLE_CLEAN,   NULL,           NULL,       NULL,               NULL,           NULL	},
    { K_POWER,          NULL,           NULL,       NULL,               NULL,           NULL   },

    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,         NULL,     NULL        	},
//    { K_MOOD,           SelMood,        NULL,           NULL,         NULL,     NULL        	},
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,         NULL,     NULL        	},

    /* Side Key */
    { K_SIDE_1,         NULL,           NULL,           NULL,         NULL,             NULL        	},
    { K_SIDE_2,         NULL,           NULL,           NULL,         NULL,             NULL        	},
    { K_SIDE_3,         NULL,           NULL,           NULL,         NULL,             NULL        	},

    /* MULTI KEY */                                     
    { K_SETTING,        NULL,           NULL,           NULL,         NULL,             NULL        	},
    { K_REMOTE_PARING,  NULL,           NULL,           NULL,         NULL,             NULL        	},
    { K_AGING,          NULL,           NULL,           NULL,         NULL,             NULL        	},
    { K_FACTORY_MODE,   SelFactoryDrain,NULL,           NULL,         NULL,             NULL        	}
};

void* GetKeyAutoTestEventList(void)
{
    return KeyAutoTestEventList;
}

U8 GetKeyAutoTestEventListSize(void)
{
    return ( sizeof(KeyAutoTestEventList) / sizeof(KeyEventList_T) );
}

static U8 SelAutoTestStart ( void )
{
    U8  mu8Sound;
    U8 mu8IsErrorTypeStop;
    U8 mu8GetFactoryAutoTest;
    U8 mu8GetSterAll;

    mu8IsErrorTypeStop = IsErrorTypeStop();
    if ( mu8IsErrorTypeStop == TRUE )
    {
        return BUZZER_NONE;
    }


    EcoPowerSaveMode_Stop();

    mu8GetFactoryAutoTest = GetFactoryAutoTest();
    mu8GetSterAll = GetSterAll();
    if ( mu8GetFactoryAutoTest == TRUE ) /* Auto Test Mode */
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        if( mu8GetSterAll == TRUE )
        {   
            Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
            EW_Stop();      //살균 중이었으면 멈추고          
        }

        SetFactoryAutoTest ( TRUE );

        mu8Sound = BUZZER_SELECT;
    }
    
    return mu8Sound;

}

static U8 SelAutoTestStop ( void )
{
    U8  mu8Sound;
    U8 mu8IsErrorTypeStop;
    U8 mu8GetSterAll;

    mu8IsErrorTypeStop = IsErrorTypeStop();
    if ( mu8IsErrorTypeStop == TRUE )
    {
        return BUZZER_NONE;
    }
    EcoPowerSaveMode_Stop();

    mu8GetSterAll = GetSterAll();
    if( mu8GetSterAll == TRUE )
    {   
        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
        EW_Stop();      //살균 중이었으면 멈추고          
    }

    Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
    Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
    Motor_State ( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
    Motor_State ( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );

    SetFactoryAutoTest ( FALSE );
    SetFactoryAutoTestStep ( AUTO_TEST_INIT );
    SetModeFlag ( MODE_FLAG_STOP );

    mu8Sound = BUZZER_SELECT;
    
    
    return mu8Sound;

}

static U8 SelWarnning ( void )
{
    U8  mu8Sound;
    U8 mu8IsErrorTypeStop;
    U8 mu8GetFactoryAutoTest;

    mu8IsErrorTypeStop = IsErrorTypeStop();        
    if ( mu8IsErrorTypeStop == TRUE )
    {
        return BUZZER_NONE;
    }
    
    EcoPowerSaveMode_Stop();

    mu8GetFactoryAutoTest = GetFactoryAutoTest();
    if ( mu8GetFactoryAutoTest == TRUE ) /* Auto Test Mode */
    {
        mu8Sound = BUZZER_WARNING;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelTestSter ( void )
{
    U8 mu8Sound;
    U8 mu8IsErrorTypeStop;
    U8 mu8GetFactoryAutoTest;

    mu8GetFactoryAutoTest = GetFactoryAutoTest();
    if ( mu8GetFactoryAutoTest == TRUE ) /* Auto Test Mode */
    {
        return BUZZER_WARNING;
    }

    mu8Sound = SelSter();   
    return mu8Sound;

}


