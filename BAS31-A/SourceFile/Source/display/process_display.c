#include "Hw.h"
#include "display.h"
#include "process_display.h"

#include "Display_Error.h"
#include "ProcessError.h"

#include "Mode.h"
#include "Buzzer.h"
#include "PowerSave.h"
#include "Setting.h"
#include "Sterilize.h"
#include "Seat.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "HotWater.h"
#include "SeatHeater.h"
#include "Dry.h"
#include "Power.h"
#include "Waterlevel.h"
#include "WaterTank.h"
#include "Eeprom.h"
#include "Aging.h"
#include "FactoryMode.h"


#include "Waterless.h"


#define LED_BLINK_PERIOD    1000    /*1mse * 1000 = 1sec*/
#define LED_BLINK_500MS     500     /*1mse * 500 = 500msec*/

#define LED_BLINK_FAST_PERIOD       200     /*1mse * 200 = 200msec*/
#define LED_BLINK_100MS             100     /*1mse * 100 = 100msec*/

#define ERROR_BUZZER_PEROID    50      /*100msec * 50 = 5sec*/
#define ERROR_LED_PEROID       10      /*100msec * 10 = 1sec*/
#define ERROR_LED_DUTY         5       /*100msec * 5 = 0.5sec*/

typedef struct _display_
{
    U8   Mode;       // NORMAL / VERSION / UART TEST / ...

    U16  BlinkTime;
    U16  u16BlinkTimeFast;

    /* Init */
    U8  Init;  /* FALSE : 초기화 전, TRUE : 초기화 완료 */

    /* Memento */
    U8 Memento;
    U8 MementoStep;
    U8 MementoTimer;
} Display_T;

Display_T   Disp;


void InitDisplay(void)
{
    //Disp.Mode = DISP_MODE_VERSION;
    Disp.BlinkTime = 0;
    Disp.u16BlinkTimeFast = 0;

    /* Booting Display - Showing Blink Front Display */
    Disp.Init = FALSE;

    INIT_LED_SCAN();

    /* TURN OFF ALL LED */
    TurnOffAllLED();
}

U8 GetDisplayInit(void)
{
    return Disp.Init;
}





/* 메멘토 설정 */
U8 GetMementoDisp(void)
{
    return Disp.Memento;
}

void SetMementoDisp(U8 mu8Conf )
{
    Disp.Memento        = mu8Conf;
    Disp.MementoStep    = 0;
    Disp.MementoTimer   = 0;
}

static void BlinkTimer(void);

static void ProcessDisplayBoot(void);

static void ProcessDisplayTestModeUART(void);
static void ProcessDisplayMementoMode(void);


static void ProcessDisplaySettingMode(void);
static void ProcessDisplayFactoryMode(void);

static void ProcessDisplayNormalMode(void);
static void ProcessDisplayErrorMode(void);


static void DisplayFactoryMode(void);

static void DisplayVersion(void);  


static void DsiplayMood(void);
static void DisplayLevel( void );
static void DisplayHotTemperature(void);
static void DisplaySeatTemperature(void);
static void DisplayState(void);
static void DisplayEW(void);




void Evt_1msec_ProcessDisplay(void)
{
    BlinkTimer();
}

void Evt_100msec_ProcessDisplay(void)
{
    if( Disp.Init == FALSE )
    {
        ProcessDisplayBoot();
        return ;
    }

    if ( GetWaterLessInspection1Start() == TRUE )
    {
        ProcessDisplayInspection1();
        return;
    }

    if ( GetWaterLessInspection2Start() == TRUE )
    {
        ProcessDisplayInspection2();
        return;
    }


    /* MEMENTO */
    if( Disp.Memento == TRUE )
    {
        ProcessDisplayMementoMode();
        return ;
    }

    /* Power OFF or PowerSave SET*/
    if ( GetPowerOnOff() == FALSE 
        || GetPowerSaveLED() == TRUE )
    {
        TurnOffAllLED();
        return;
    }

    /* 공장 잔수*/
    if ( GetFactoryStart() == TRUE )
    {
        ProcessDisplayFactoryMode();
        return;
    }
    
    /*특수모드에서 LED 제어 */
    if ( GetSettingTime() != 0 )
    {
        ProcessDisplaySettingMode();
        return;
    }
    
    /* Normal Mode */
    if ( IsErrorTypeStop() == TRUE )
    {
        /* ERRROR MODE */
        ProcessDisplayErrorMode();
    }
    else
    {
        /* NORMAL MODE */
        ProcessDisplayNormalMode();
    }
}




static void BlinkTimer(void)
{
    /* Update Blink Timer */
    if( Disp.BlinkTime < LED_BLINK_PERIOD )
    {
        Disp.BlinkTime++;
    }
    else
    {
        Disp.BlinkTime = 0;
    }
    
    if( Disp.u16BlinkTimeFast < LED_BLINK_FAST_PERIOD )
    {
        Disp.u16BlinkTimeFast++;
    }
    else
    {
        Disp.u16BlinkTimeFast = 0;
    }

    
}

static void ProcessDisplayFactoryMode(void)
{
    DisplayFactoryMode();

    /* Mood */
    DsiplayMood();
}


static void ProcessDisplayNormalMode(void)
{
    U8 mu8GetFactoryTest_Remote;
    //U8 mu8GetFactoryAutoTest;

    mu8GetFactoryTest_Remote = GetFactoryTest_Remote();
    //mu8GetFactoryAutoTest = GetFactoryAutoTest();

    /*Level*/
    DisplayLevel();

    /*Hot Temperature*/
    DisplayHotTemperature();

    /*Seat Temperature*/
    DisplaySeatTemperature();

    /* State */
    /* 공장 Auto Test EEPROM Check */
    if ( mu8GetFactoryTest_Remote == TRUE )
        //|| mu8GetFactoryAutoTest == TRUE )
    {
        DispAutoTestMode_State();
    }
    else
    {
        DisplayState();
    }
    
    /* Mood */
    DsiplayMood();

    /* Nozzle Clean */
    DisplayEW();
}

/* 디스플레이 초기화 표시 */
static void ProcessDisplayBoot(void)
{
    static U8 mu8Count = 35;

    mu8Count--;

    /* 3sec 점멸 */
    if ( mu8Count > 5)
    {
        if( Disp.BlinkTime < LED_BLINK_500MS )
        {
            TurnOffAllLED();
        }
        else
        {
            TurnOnAllLED();
        }
    }
    /* 0.5sec version display */
    else if ( mu8Count > 0 
        && mu8Count < 6 )
    {
        DisplayVersion();
    }
    else    //if( mu8Count == 0 )
    {
        Disp.Init = TRUE;
        TurnOffAllLED();
        
        return;
    }
}

static void ProcessDisplayTestModeUART(void)
{
#if 0
    TestUartDisplay();
#endif
}

static void ProcessDisplayMementoMode(void)
{
    ErrorId_T  mError;


    if( Disp.MementoTimer != 0 )
    {
        Disp.MementoTimer--;
        return ;
    }

    // 1. ALL ON LED - 1 SEC
    // 2. ALL OFF LED - 1 SEC
    // 3. DISPLAY ERROR 5SEC & SOUND 1~5
    // 3. DISPLAY ERROR 5SEC & SOUND 1
    switch( Disp.MementoStep )
    {
        case 0 : 
            TurnOnAllLED();	
            Disp.MementoTimer = 10;

            Disp.MementoStep++;
            break;

        case 1:
            TurnOffAllLED();
            Disp.MementoTimer = 10;

            Disp.MementoStep++;
            break;

        case 2:
            mError = GetMementoError( 4 );
            if( mError == ERR_NONE )
            {
                Disp.MementoStep = 0;

            }
            else
            {
                DisplayError( mError, ON );
                BUZZER_STEP(BUZZER_MEMENTO_1);
                Disp.MementoTimer = 50;

                Disp.MementoStep++;
            }
            break;

        case 3:
            TurnOffAllLED();
            Disp.MementoTimer = 10;
            Disp.MementoStep++;
            break;

        case 4:
            mError = GetMementoError( 3 );
            if( mError == ERR_NONE )
            {
                Disp.MementoStep = 0;

            }
            else
            {
                DisplayError( mError, ON );
                BUZZER_STEP(BUZZER_MEMENTO_2);
                Disp.MementoTimer = 50;

                Disp.MementoStep++;
            }
            break;

        case 5:
            TurnOffAllLED();
            Disp.MementoTimer = 10;
            Disp.MementoStep++;
            break;

        case 6:
            mError = GetMementoError( 2 );
            if( mError == ERR_NONE )
            {
                Disp.MementoStep = 0;

            }
            else
            {
                DisplayError( mError, ON );
                BUZZER_STEP(BUZZER_MEMENTO_3);
                Disp.MementoTimer = 50;

                Disp.MementoStep++;
            }
            break;

        case 7:
            TurnOffAllLED();
            Disp.MementoTimer = 10;
            Disp.MementoStep++;
            break;

        case 8:
            mError = GetMementoError( 1 );
            if( mError == ERR_NONE )
            {
                Disp.MementoStep = 0;

            }
            else
            {
                DisplayError( mError, ON );
                BUZZER_STEP(BUZZER_MEMENTO_4);
                Disp.MementoTimer = 50;

                Disp.MementoStep++;
            }
            break;

        case 9:
            TurnOffAllLED();
            Disp.MementoTimer = 10;
            Disp.MementoStep++;
            break;

        case 10:
            mError = GetMementoError( 0 );
            if( mError == ERR_NONE )
            {
                Disp.MementoStep = 0;

            }
            else
            {
                DisplayError( mError, ON );
                BUZZER_STEP(BUZZER_MEMENTO_5);
                Disp.MementoTimer = 50;

                Disp.MementoStep++;
            }

            break;

        case 11:
            TurnOffAllLED();
            Disp.MementoTimer = 10;
            Disp.MementoStep = 0;
            break;

    }

    return ;
}

extern U8 gu8SettingLEDBit;

static void ProcessDisplaySettingMode(void)
{
    U8 mu8OnOff;

    /* State (Mode) LED OFF*/
    DispState ( STATE_NONE );
    DispMood ( OFF, OFF );
    
    /* 노즐클린 LED 점멸 */
    if ( Disp.BlinkTime < LED_BLINK_500MS )
    {
        mu8OnOff = ON;
    }
    else
    {
        mu8OnOff = OFF;
    }
    
    DispEW ( mu8OnOff );

    
    // sound
    if ( GetBuzOnOff() == TRUE )
    {
        SetDispSetting ( SETTING_BIT_MUTE ); 
    }
    else
    {
        ClearDispSetting ( SETTING_BIT_MUTE ); 
    }

    // seat
    if ( GetSeatSetting() == TRUE )
    {
        SetDispSetting ( SETTING_BIT_SEAT ); 
    }
    else
    {
        ClearDispSetting ( SETTING_BIT_SEAT ); 
    }

    // auto drain
    if ( GetAutoDrainSetting() == TRUE )
    {
        SetDispSetting ( SETTING_BIT_AUTO_DRAIN ); 
    }
    else
    {
        ClearDispSetting ( SETTING_BIT_AUTO_DRAIN ); 
    }

    //body
    if ( GetSterSettingBody() == TRUE )
    {
        SetDispSetting ( SETTING_BIT_STERILIZE_BODY ); 
    }
    else
    {
        ClearDispSetting ( SETTING_BIT_STERILIZE_BODY ); 
    }

    // PowerSave
    if ( GetPowerSaveMode() == TRUE )
    {
        SetDispSetting ( SETTING_BIT_POWER_SAVE ); 
    }
    else
    {
        ClearDispSetting ( SETTING_BIT_POWER_SAVE ); 
    }


    DispSetting ( gu8SettingLEDBit );
}



static void ProcessDisplayErrorMode(void)
{
    static ErrorId_T mu8PrevErr = 0;
    ErrorId_T mu8CurErr = 0;

    static U8 mu8Cmd = 0;
    static U8 mu8BuzzerCounter = ERROR_BUZZER_PEROID;
    static U8 mu8LEDCounter = ERROR_LED_PEROID;


    /* Get error code */
    mu8CurErr = GetErrorId();

    /* Display error code */
    if( mu8CurErr == ERR_NONE )
    {
        mu8Cmd = 0;
        mu8BuzzerCounter = ERROR_BUZZER_PEROID;
        mu8LEDCounter = ERROR_LED_PEROID;
    }
    else
    {
        /* Warnning Buzzer */
        mu8BuzzerCounter--;
        
        if( mu8BuzzerCounter == 0 )
        {
            mu8BuzzerCounter = ERROR_BUZZER_PEROID;
            if ( mu8CurErr != ERR_WATER_SUPPLY_CRITICAL )
            {
                BUZZER_STEP( BUZZER_WARNING );
            }
        }

        /* LED Display */
        mu8LEDCounter--;

        if( mu8LEDCounter == 0 )
        {
            mu8LEDCounter = ERROR_LED_PEROID;
        }

        if( mu8LEDCounter < ERROR_LED_DUTY )
        {
            mu8Cmd = 0;
        }
        else
        {
            mu8Cmd = 1;
        }

        DisplayError( mu8CurErr, mu8Cmd );
    }

    /* New error code */
    if( mu8PrevErr != mu8CurErr )
    {
        mu8PrevErr = mu8CurErr;
    }
}













static void DisplayVersion( void )
{
    DispVersion ( VERSION );
}

static void DisplayFactoryMode(void)
{

    U8 mu8OnOff_G = OFF;
    U8 mu8OnOff_R = OFF;

    if ( GetFactoryLED() == FACTORY_LED_1 )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff_G = ON;
            mu8OnOff_R = OFF;
        }
        else
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = OFF;
        }
    }
    else if ( GetFactoryLED() == FACTORY_LED_2 )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff_G = ON;
            mu8OnOff_R = ON;
        }
        else
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = OFF;
        }

    }
    else if ( GetFactoryLED() == FACTORY_LED_3 )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = ON;
        }
        else
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = OFF;
        }

    }
    else //if ( GetFactoryLED() == FACTORY_LED_0 )
    {
        mu8OnOff_G = OFF;
        mu8OnOff_R = OFF;
    }

    DispHotTemperature( mu8OnOff_G, mu8OnOff_R);
    DispSeatTemperature( mu8OnOff_G, mu8OnOff_R);

}


static void DisplayEW(void)
{
    U8 mu8OnOff = OFF;

    if ( GetModeSel() == TYPE_DRY )
    {
        DispEW(mu8OnOff);
        return;
    }

    /* 2018-10-05 안용인님..PBA입고시..EEPROM..동작여부..검사..정상동작시..노즐클린..LED..점등..*/
    if ( GetFactoryTest() == TRUE )
    {
        if ( GetEepormInit() == TRUE )
        {
            mu8OnOff = ON;
        }
        else
        {
            mu8OnOff = OFF;
        }
        
        DispEW(mu8OnOff);
        return;
    }

    /* 조작부 위 노즐 클린 LED 점멸 조건 */
    /* 1. 착좌감지 노즐 세척 */
    /* 2. 사용 후 후세척 */
    /* 3. 전체 살균 시 */
    /* 4. 노즐세척 버튼 입력하여 세척 시 */

    if ( GetSterAll() == TRUE
        || GetSterAfter() == TRUE )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff = ON;
        }
        else
        {
            mu8OnOff = OFF;
        }
    }
    else
    {
        mu8OnOff = OFF;
    }


    /* 노즐 세척 시 노즐 클린 LED 점멸 */
    if ( GetPowerOnOff() == TRUE 
        && GetNozzleClean() == TRUE 
        && GetAutoDrainStart() == FALSE )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff = ON;
        }
        else
        {
            mu8OnOff = OFF;
        }
    }

#if 0
    /* 공장 테스트 모드 중 EW 모듈 동작 시 LED ON */
    if ( GetFactoryTest() == TRUE 
        && GET_STATUS_STER() == TRUE )
    {
        mu8OnOff = ON;
    }
#endif
    DispEW(mu8OnOff);

}


static void DsiplayMood(void)
{
    U8 mu8OnOff_B = OFF;
    U8 mu8OnOff_R = OFF;
    
    /* Mood BLUE LED 점등 조건 */
    /* 1. 전체 살균 시 */
    /* 사용 후 후세척OFF */
    /* 착좌감지 노즐세척시 OFF */
    
    
    if ( GetSterAll() == TRUE )
    {
        mu8OnOff_B = ON;
        mu8OnOff_R = OFF;
    }
    /* 살균 완료시 5sec간 점멸 */
    else if ( GetSterDisplayFinishBlinkTime() != 0 )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff_B = ON;
            mu8OnOff_R = OFF;
        }
        else
        {
            mu8OnOff_B = OFF;
            mu8OnOff_R = OFF;
        }
    }
    /* 살균 완료 후 5sec 경과시 점등 */
    else if ( GetSterDisplayFinishSeatTime() != 0 )
    {
        mu8OnOff_B = ON;
        mu8OnOff_R = OFF;
    }
    /* normal Mode */
    /* 살균 완료 후 착좌시 소등 */
    else
    {
        /* 수동 테스트 모드 */
        if ( GetFactoryTest() == TRUE )
        {
            if ( Disp.BlinkTime < LED_BLINK_500MS )
            {
                mu8OnOff_B = ON;
                mu8OnOff_R = OFF;
            }
            else
            {
                mu8OnOff_B = OFF;
                mu8OnOff_R = OFF;
            }  
        }
        /* 자동 테스트 모드 */
        else if ( GetFactoryTest_Remote() == TRUE )
        {
            /* 자동 테스트 모드 진입 */
            if ( GetFactoryAutoTest() == TRUE )
            {
                if ( Disp.u16BlinkTimeFast < LED_BLINK_100MS )
                {
                    mu8OnOff_B = ON;
                    mu8OnOff_R = OFF;
                }
                else
                {
                    mu8OnOff_B = OFF;
                    mu8OnOff_R = OFF;
                }
            }
            /* 자동 테스트 모드 대기 */
            else
            {
                if ( Disp.BlinkTime < LED_BLINK_500MS )
                {
                    mu8OnOff_B = ON;
                    mu8OnOff_R = OFF;
                }
                else
                {
                    mu8OnOff_B = OFF;
                    mu8OnOff_R = OFF;
                }  
            }
        }
        else
        {
            mu8OnOff_B = OFF;
            mu8OnOff_R = OFF;
        }
    }

    DispMood( mu8OnOff_B, mu8OnOff_R);

}



static void DisplayLevel( void )
{
    U8 mu8Level = 0;

    
    /* Soft Start 시 수압 LED 강제 1단 표시 */
#if 0
    if ( ( ( ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE ) 
            && ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == FALSE ) )
        || ( ( IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) 
            && ( IsSetBidetMode( BIDET_MODE_BEFORE ) == FALSE ) ) )
        && ( GetMainStep() == STEP_MAIN_START_SETTING
            || GetMainStep() == STEP_MAIN_START_ING ) )
    {
        mu8Level = 1; 
    }
    else
    {
        mu8Level = GetWaterPressLevel(); 
    }
#endif

    /* Soft Start 시 수압 LED 사용자 설정 표시 */
    mu8Level = GetWaterPressLevel(); 

    DispLevel ( mu8Level );    

}

static void DisplayHotTemperature(void)
{
    U8 mu8OnOff_G = OFF;
    U8 mu8OnOff_R = OFF;

    //평소엔 온수온도 건조할땐 건조온도
    if ( GetModeSel() == TYPE_DRY )
    {
        if ( GetDryService() == FALSE )
        {
            if ( GetDryLevel() == 1 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = OFF;
            }
            else if ( GetDryLevel() == 2 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = ON;
            }
            else if ( GetDryLevel() == 3 )
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = ON;
            }
            else
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = OFF;
            }
        }
        //건조 서비스 모드에서는 OFF
        else
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = OFF;
        }
    }
    //일반모드
    else
    {
        if ( GetHotPreHeating_LED() == TRUE )
        {
            if ( Disp.BlinkTime < LED_BLINK_500MS )
            {
                if ( GetHotLevel() == 1 )
                {
                    mu8OnOff_G = ON;
                    mu8OnOff_R = OFF;
                }
                else if ( GetHotLevel() == 2 )
                {
                    mu8OnOff_G = ON;
                    mu8OnOff_R = ON;
                }
                else if ( GetHotLevel() == 3 )
                {
                    mu8OnOff_G = OFF;
                    mu8OnOff_R = ON;
                }
                else 
                {
                    mu8OnOff_G = OFF;
                    mu8OnOff_R = OFF;
                }
            }
            else
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = OFF;
            }

        }
        else
        {
            if ( GetHotLevel() == 1 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = OFF;
            }
            else if ( GetHotLevel() == 2 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = ON;
            }
            else if ( GetHotLevel() == 3 )
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = ON;
            }
            else 
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = OFF;
            }
        }
    }

    DispHotTemperature( mu8OnOff_G, mu8OnOff_R);
}

static void DisplaySeatTemperature(void)
{
    U8 mu8OnOff_G = OFF;
    U8 mu8OnOff_R = OFF;

    if ( GetFactoryReady() == TRUE )
    {
        /* 2018-08-31 최정인님. Factory Drain Ready LED 표시 */
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = ON;
        }
        else
        {
            mu8OnOff_G = OFF;
            mu8OnOff_R = OFF;
        }
    }
    else
    {
        // 절전모드
        if ( GetPowerSaveStart() == TRUE )
        {
            if ( GetSeatHeaterLevel() != FALSE )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = OFF;
            }
            else
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = OFF;
            }
        }
        // 일반모드
        else
        {
            if ( GetSeatHeaterLevel() == 1 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = OFF;
            }
            else if ( GetSeatHeaterLevel() == 2 )
            {
                mu8OnOff_G = ON;
                mu8OnOff_R = ON;
            }
            else if ( GetSeatHeaterLevel() == 3 )
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = ON;
            }
            else 
            {
                mu8OnOff_G = OFF;
                mu8OnOff_R = OFF;
            }
        }
    }
    

    DispSeatTemperature( mu8OnOff_G, mu8OnOff_R);
}

static void DisplayState(void)
{
    U8 mu8State = 0;


    /* Aging Mode 진입시 해당 Mode 점멸 */
    if ( GetAgingStart() == TRUE )
    {
        if ( Disp.BlinkTime < LED_BLINK_500MS )
        {
            mu8State = GetModeState();     
        }
        else
        {
            mu8State = STATE_NONE;
        }
    }
    else
    {
        mu8State = GetModeState();    
    }

    DispState ( mu8State );
}



