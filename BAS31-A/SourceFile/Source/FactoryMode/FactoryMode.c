#include "hw.h"

#include "FactoryMode.h"
#include "Mode.h"
#include "ProcessError.h"
#include "SteppingMotor.h"
#include "PowerSave.h"
#include "Valve.h"

#include "WaterPress.h"
#include "Dry.h"
#include "Hotwater.h"
#include "Seat.h"
#include "Seatheater.h"
#include "Air_plus.h"
#include "Setting.h"
#include "Buzzer.h"
#include "Sterilize.h"
#include "Nozzle.h"
#include "Eeprom.h"
#include "Waterlevel.h"
#include "WaterTank.h"


#include "Waterless.h"


#include "Hal_key.h"

/*************************************************************************
FILE: FactoryMode.c

%Header: FactoryMode.h %

DESCRIPTION:
 *  -. 공장잔수빼기 scheduler
 *  -. 공장테스트모드 scheduler

 Note.

Copyright (c) 2019 - Coway - All rights reserved.

%History: %
 2019.07.29 김민환 초안
**************************************************************************/

typedef enum
{
//LED1
    FACTORY_INITIAL,
    FACTORY_SAVE,
    FACTORY_VALVE_OFF_SELF,

    FACTORY_MOVE_POTTERY,
    FACTORY_VALVE_ON_POTTERY,
    FACTORY_PUMP_ON_POTTERY,
    FACTORY_PUMP_OFF_POTTERY,

//LED2    
    FACTORY_MOVE_BIDET,
    FACTORY_VALVE_ON_BIDET,
    FACTORY_VALVE_OFF_BIDET,
//LED3   
    FACTORY_MOVE_CLEAN,
    FACTORY_VALVE_ON_CLEAN,
    FACTORY_VALVE_OFF_CLEAN,
    FACTORY_MOVE_ORIGIN,
    FACTORY_END,
    
} FactoryStartId_T;

#define AUTO_TEST_1SEC      10              /* 100msec * 10 = 1sec */
#define AUTO_TEST_3SEC      30              /* 100msec * 30 = 3sec */


#define FACTORY_DELAY               20     /* 2sec */
#define FACTORY_KEY_COUNT           10     /* 10msec 이내에 TEST 모드 진입 가능 */

typedef struct _factory_
{
    /* Factory Drain */
    U8 u8Start;             /* 공장 Drain */
    U8 u8Ready;             /* 공장 Drain Ready flag */
    U8 u8Step;              /* 공장 Drain Step */
    U16 u16Delay;           /* 공장 Drain Delay */
    U8 u8LED;               /* 공장 Drain Step별 LED 표시 */


    /* 공장 TEST Mode 진입 조건 */
    U8 Key;             /* Init SET, KeyCount 종료 후 CLR */
    U16 KeyCount;       /* Key 입력시간 10msec */


    /* 공장 수동 TEST */
    U8 u8Test;            

    
    /* 공장 자동 TEST */
    U8 u8Test_Remote;             /* 공장 Auto Test 모드 진입 */
    U8 u8AutoTest;                /* 공장 Auto Test 모드 시작 */
    U8 u8AutoTestStep;            /* Auto Test 시퀀스 */
    U16 u16AutoTestTime;          /* 공장 Auto Test Control Time */

    

} Factory_T;

Factory_T Factory;

void InitFactory ( void )
{
    Factory.u8Start = FALSE;        
    Factory.u8Ready = FALSE;        
    Factory.u8Step = FACTORY_INITIAL;
    Factory.u16Delay = 0;
    Factory.u8LED = FACTORY_LED_0;

    Factory.Key = TRUE;
    Factory.KeyCount = FACTORY_KEY_COUNT;

    Factory.u8Test = FALSE;

    Factory.u8Test_Remote = FALSE;
    Factory.u8AutoTest = FALSE;
    Factory.u8AutoTestStep = AUTO_TEST_INIT;
    Factory.u16AutoTestTime = 0;
}

void SetFactoryStart ( U8 mu8Val)
{
    Factory.u8Start = mu8Val;
}

U8 GetFactoryStart ( void )
{
    return Factory.u8Start;
}

void SetFactoryReady ( U8 mu8Val)
{
    Factory.u8Ready = mu8Val;
}

U8 GetFactoryReady ( void )
{
    return Factory.u8Ready;
}


void SetFactoryLED ( U8 mu8Val)
{
    Factory.u8LED = mu8Val;
}

U8 GetFactoryLED ( void )
{
    return Factory.u8LED;
}

void SetFactoryTest ( U8 mu8Val)
{
    Factory.u8Test = mu8Val;
}

U8 GetFactoryTest ( void )
{
    return Factory.u8Test;
}

void SetFactoryTest_Remote ( U8 mu8Val)
{
    Factory.u8Test_Remote = mu8Val;
}

U8 GetFactoryTest_Remote ( void )
{
    return Factory.u8Test_Remote;
}


void SetFactoryAutoTest ( U8 mu8Val)
{
    Factory.u8AutoTest = mu8Val;
}

U8 GetFactoryAutoTest ( void )
{
    return Factory.u8AutoTest;
}


void SetFactoryAutoTestStep ( U8 mu8Val)
{
    Factory.u8AutoTestStep = mu8Val;
}

U8 GetFactoryAutoTestStep ( void )
{
    return Factory.u8AutoTestStep;
}








void Evt_100msec_Factory_Handler(void)
{
    U8 mu8GetModeSel;
    U8 mu8IsErrorTypeStop;
    U8 mu8IsSetMotorEndFlagPress;
    U8 mu8IsSetMotorEndFlagWide;
    U8 mu8GetValveOnOff;
    U16 mu16GetValveOffTime;

    mu8GetModeSel = GetModeSel();

    if ( mu8GetModeSel != TYPE_WAIT
        || Factory.u8Start == FALSE )
    {
        return;
    }

    /*Err가 없으면 */
    mu8IsErrorTypeStop = IsErrorTypeStop();
    mu8IsSetMotorEndFlagPress = IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS);
    mu8IsSetMotorEndFlagWide = IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE);
    if ( mu8IsErrorTypeStop == FALSE )
    {
        switch ( Factory.u8Step )
        {
            //공장잔수빼기 초기동작
            case FACTORY_INITIAL :
                Factory.u8LED = FACTORY_LED_1;

                SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
                Factory.u8Step++;
            break;
        
            /*EEPROM Save*/
            case FACTORY_SAVE:
                if( mu8IsSetMotorEndFlagPress == TRUE 
                    && mu8IsSetMotorEndFlagWide == TRUE )
                {
                    SetBuzOnOff( TRUE );
                    SetSterSettingBody ( TRUE );
                    SetSeatSetting ( TRUE );
                    SetWaterLevelInitial ( TRUE );
                    SetAutoDrainSetting ( FALSE );
                    SetPowerSaveMode( POWER_SAVE_SET );
                    SetModeStateDefault ( STATE_BASIC );
                    SetModeState ( STATE_BASIC );
                    ClearMementoError();
                    //SetSettingSaveMode ( FALSE );
                    //SetWaterPressLevel(3);
                    //SetNozzleLevel(3);
                    //SetSettingMood( FALSE );

                    //EEPROM
                    WriteData();
            
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    Factory.u16Delay = FACTORY_DELAY;
                    Factory.u8Step++;
                }
            break;

            case FACTORY_VALVE_OFF_SELF:
                if ( Factory.u16Delay != 0)
                {
                    Factory.u16Delay--;
                }
                else
                {
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                    Factory.u8Step++;
                }
            break;

            /* Valve OFF -> POTTERY */
            case FACTORY_MOVE_POTTERY:
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_POTTERY, (U8) TYPE_BIT_MOTOR_PRESS );    
                Factory.u8Step++;
            break;

            /* POTTERY -> Valve ON */
            case FACTORY_VALVE_ON_POTTERY:
                if ( mu8IsSetMotorEndFlagPress == TRUE )
                {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    Factory.u16Delay = 10;
                    Factory.u8Step++;
                }
            break;

            /* POTTERY -> Water Pump ON */
            case FACTORY_PUMP_ON_POTTERY:
                if ( Factory.u16Delay != 0)
                {
                    Factory.u16Delay--;
                }
                else
                {
                    TURN_ON_WATER_PUMP();
                    Factory.u16Delay = 40;
                    Factory.u8Step++;
                }
            break;

            /* POTTERY -> Water Pump OFF */
            case FACTORY_PUMP_OFF_POTTERY:
                if ( Factory.u16Delay != 0)
                {
                    Factory.u16Delay--;
                }
                else
                {
                    TURN_OFF_WATER_PUMP();
                    SetValveOnOff( FALSE, VALVE_DELAY_1SEC );
                    Factory.u8Step++;
                }
            break;

            /* Pump OFF -> Bidet */
            case FACTORY_MOVE_BIDET:
                mu8GetValveOnOff = GetValveOnOff();
                mu16GetValveOffTime = GetValveOffTime();
                if ( mu8GetValveOnOff == FALSE 
                    && mu16GetValveOffTime == 0 )
                {
                    Factory.u8LED = FACTORY_LED_2;
                    
                    Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_3, (U8) TYPE_BIT_MOTOR_PRESS );    
                    Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_BIDET_2, (U8) TYPE_BIT_MOTOR_WIDE );
                    Factory.u8Step++;
                }
            break;
        
            /* Care Bidet -> Valve ON*/
            case FACTORY_VALVE_ON_BIDET:
                if ( mu8IsSetMotorEndFlagPress == TRUE 
                   && mu8IsSetMotorEndFlagWide == TRUE )
                {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    Factory.u16Delay = FACTORY_DELAY;
                    Factory.u8Step++;
                }
            break;

            case FACTORY_VALVE_OFF_BIDET:
                if ( Factory.u16Delay != 0)
                {
                    Factory.u16Delay--;
                }
                else
                {
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                    Factory.u8Step++;
                }
            break;
            
            /*Move Clean*/
            case FACTORY_MOVE_CLEAN:
                Factory.u8LED = FACTORY_LED_3;
                
                Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_CLEAN_1, (U8) TYPE_BIT_MOTOR_WIDE );    
                Factory.u8Step++;
            break;
            
            /*Move Clean -> Valve ON*/
            case FACTORY_VALVE_ON_CLEAN:
                if ( mu8IsSetMotorEndFlagWide == TRUE )
                {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    Factory.u16Delay = FACTORY_DELAY;
                    Factory.u8Step++;
                }
            break;
            
            case FACTORY_VALVE_OFF_CLEAN:
                if ( Factory.u16Delay != 0)
                {
                    Factory.u16Delay--;
                }
                else
                {
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                    Factory.u8Step++;
                }
            break;
            
            case FACTORY_MOVE_ORIGIN:
                Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );   
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Factory.u8Step++;
            break;

            case FACTORY_END:
                if ( mu8IsSetMotorEndFlagPress == TRUE
                    && mu8IsSetMotorEndFlagWide == TRUE )
                {
                    Factory.u8LED = FACTORY_LED_0;
                    
                    Factory.u8Step = FACTORY_INITIAL;
                    Factory.u8Start = FALSE;          
                    BUZZER_STEP ( BUZZER_SET_OFF );       //잔수빼기 OFF
                 }
            break;
        }
    }
    //2014-11-03 김일욱  잔수 빼기 중 에러발생시 잔수빼기 정지 후 에러 표시
    else     
    {
        Factory.u8LED = FACTORY_LED_0;
        Factory.u8Step = FACTORY_INITIAL;
        Factory.u8Start = FALSE;
      
        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );    
        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );    
        SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
    }

}

   
void Evt_100msec_Test_Handler(void)
{
    U8 mu8GetEepormInit;

    if ( Factory.u8Test == FALSE )
    {
        return;
    }

    /* PBA업체에서..Write Data Check */
    /* 2018-10-05 안용인님..PBA입고시..EEPROM..동작여부..검사..정상동작시..노즐클린..LED..점등..*/
    mu8GetEepormInit = GetEepormInit();
    if ( mu8GetEepormInit == FALSE )
    {
        LoadData();
    }
}






void Evt_100msec_AutoTest_Handler(void)
{
    U8 mu8GetFactoryTest_Remote;
    U8 mu8GetEepormInit;
    U8 mu8IsSetMotorFirstFlagClean;
    U8 mu8IsSetMotorFirstFlagBidet;
    U8 mu8IsSetMotorFirstFlagPress;
    U8 mu8IsSetMotorFirstFlagWide;
    U16 mu16GetWaterLevelAddWaterTime;
    U8 mu8GetWaterLevelInitialStart;
    U8 mu8GetWaterLevel;
    U8 mu8GetWaterLessInspection1Start;
    U8 mu8GetWaterLessInspection2Start;
    U8 mu8GetFactoryAutoTest;
    U8 mu8IsSetMotorEndFlagClean;
    U8 mu8IsSetMotorEndFlagBidet;
    U8 mu8IsSetMotorEndFlagPress;
    U8 mu8IsSetMotorEndFlagWide;

    mu8GetFactoryTest_Remote = GetFactoryTest_Remote();
    if ( mu8GetFactoryTest_Remote == FALSE )
    {
        return;
    }

    /* PBA업체에서..Write Data Check */
    /* 2018-10-05 안용인님..PBA입고시..EEPROM..동작여부..검사..정상동작시..Side..LED..점등..*/
    mu8GetEepormInit = GetEepormInit();
    if ( mu8GetEepormInit == FALSE )
    {
        LoadData();
    }

    mu8IsSetMotorFirstFlagClean = IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN);
    mu8IsSetMotorFirstFlagBidet = IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET);
    mu8IsSetMotorFirstFlagPress = IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS);
    mu8IsSetMotorFirstFlagWide = IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE);
    mu16GetWaterLevelAddWaterTime = GetWaterLevelAddWaterTime();
    mu8GetWaterLevelInitialStart = GetWaterLevelInitialStart();
    mu8GetWaterLevel = GetWaterLevel();
    mu8GetWaterLessInspection1Start = GetWaterLessInspection1Start();
    mu8GetWaterLessInspection2Start = GetWaterLessInspection2Start();
    
    if ( mu8IsSetMotorFirstFlagClean == TRUE
        || mu8IsSetMotorFirstFlagBidet == TRUE
        || mu8IsSetMotorFirstFlagPress == TRUE
        || mu8IsSetMotorFirstFlagWide == TRUE
        || mu16GetWaterLevelAddWaterTime != 0
        || mu8GetWaterLevelInitialStart == TRUE
        || mu8GetWaterLevel == FALSE
        || mu8GetWaterLessInspection1Start == TRUE
        || mu8GetWaterLessInspection2Start == TRUE )
    {
        return;
    }

    mu8GetFactoryAutoTest = GetFactoryAutoTest();    
    if ( mu8GetFactoryAutoTest == FALSE )
    {
        return;
    }

    mu8IsSetMotorEndFlagClean = IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN);
    mu8IsSetMotorEndFlagBidet = IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET);
    mu8IsSetMotorEndFlagPress = IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS);
    mu8IsSetMotorEndFlagWide = IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE);
    switch ( Factory.u8AutoTestStep )
    {
        case AUTO_TEST_INIT:

            Factory.u8AutoTestStep++;

			
        break;

        /* 세정 노즐 동작 */
        case AUTO_TEST_CLEAN_INIT:

            SetModeState ( STATE_BASIC );
            
            SetDryLevel ( 3 );
            SetHotLevel ( 3 );
            SetSeatHeaterLevel ( 0 );
            
            SetWaterPressLevel ( PRESS_MINIUM );
            SetWaterPressLevelTemp ( PRESS_MINIUM );
            SetNozzleLevel ( NOZZLE_MAXIUM );
            SetNozzleLevelTemp ( NOZZLE_MAXIUM );
            
            Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_1, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_CLEAN_1, (U8) TYPE_BIT_MOTOR_WIDE );
            Motor_State ( (U8) TYPE_MOTOR_CLEAN, NOZZLE_MAXIUM, (U8) TYPE_BIT_MOTOR_CLEAN );
            
            Factory.u16AutoTestTime = AUTO_TEST_1SEC;
            Factory.u8AutoTestStep++;

        break;

        /* 밸브 ON, Air */
        case AUTO_TEST_CLEAN_START:
            if ( mu8IsSetMotorEndFlagClean == TRUE
                && mu8IsSetMotorEndFlagBidet == TRUE
                && mu8IsSetMotorEndFlagPress == TRUE
                && mu8IsSetMotorEndFlagWide == TRUE )
            {
                SetAirPlusCmd ( TRUE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    SetWaterPressLevel ( PRESS_MAXIUM );
                    SetWaterPressLevelTemp ( PRESS_MAXIUM );     
                    Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_3, (U8) TYPE_BIT_MOTOR_PRESS );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;


        /* 수압 변경, Air */
        case AUTO_TEST_CLEAN_PRESS:
            if ( mu8IsSetMotorEndFlagPress == TRUE )
            {
                SetAirPlusCmd ( TRUE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_CLEAN_2, (U8) TYPE_BIT_MOTOR_WIDE );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;


        /* Wide 변경, Air */
        case AUTO_TEST_CLEAN_WIDE:
            if ( mu8IsSetMotorEndFlagWide == TRUE )
            {
                SetAirPlusCmd ( TRUE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    SetNozzleLevel ( NOZZLE_MINIUM );
                    SetNozzleLevelTemp ( NOZZLE_MINIUM );

                    Motor_State ( (U8) TYPE_MOTOR_CLEAN, NOZZLE_MINIUM, (U8) TYPE_BIT_MOTOR_CLEAN );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* 노즐 변경, Air */
        case AUTO_TEST_CLEAN_NOZZLE:
            if ( mu8IsSetMotorEndFlagClean == TRUE )
            {
                SetAirPlusCmd ( TRUE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    Factory.u16AutoTestTime = AUTO_TEST_3SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* Delay */
        case AUTO_TEST_CLEAN_DELAY:
            if ( Factory.u16AutoTestTime != 0 )
            {
                Factory.u16AutoTestTime--;
                SetAirPlusCmd ( TRUE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
            }
            else
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );


                Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                Factory.u8AutoTestStep++;
            }
        break;

        /* 밸브 OFF */
        case AUTO_TEST_CLEAN_END:
            if ( Factory.u16AutoTestTime != 0 )
            {
                Factory.u16AutoTestTime--;
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
            }
            else
            {
                Motor_State ( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );

                Factory.u8AutoTestStep++;
            }
        break;

        /* 비데 동작 */
        case AUTO_TEST_BIDET_INIT:
            SetModeState ( STATE_BASIC );
            
            SetDryLevel ( 3 );
            SetHotLevel ( 3 );
            SetSeatHeaterLevel ( 0 );

            SetWaterPressLevel ( PRESS_MINIUM );
            SetWaterPressLevelTemp ( PRESS_MINIUM );
            SetNozzleLevel ( NOZZLE_MAXIUM );
            SetNozzleLevelTemp ( NOZZLE_MAXIUM );

            Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_1, (U8) TYPE_BIT_MOTOR_PRESS );
            Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_BIDET_1, (U8) TYPE_BIT_MOTOR_WIDE );
            Motor_State ( (U8) TYPE_MOTOR_BIDET, NOZZLE_MAXIUM, (U8) TYPE_BIT_MOTOR_BIDET );

            Factory.u16AutoTestTime = AUTO_TEST_1SEC;
            Factory.u8AutoTestStep++;
			
        break;

        /* 밸브 ON */
        case AUTO_TEST_BIDET_START:
            if ( mu8IsSetMotorEndFlagClean == TRUE
                && mu8IsSetMotorEndFlagBidet == TRUE
                && mu8IsSetMotorEndFlagPress == TRUE
                && mu8IsSetMotorEndFlagWide == TRUE )
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    SetWaterPressLevel ( PRESS_MAXIUM );
                    SetWaterPressLevelTemp ( PRESS_MAXIUM );     
                    Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_3, (U8) TYPE_BIT_MOTOR_PRESS );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* 수압 변경 */
        case AUTO_TEST_BIDET_PRESS:
            if ( mu8IsSetMotorEndFlagPress == TRUE )
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_BIDET_2, (U8) TYPE_BIT_MOTOR_WIDE );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* Wide 변경 */
        case AUTO_TEST_BIDET_WIDE:
            if ( mu8IsSetMotorEndFlagWide == TRUE )
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    SetNozzleLevel ( NOZZLE_MINIUM );
                    SetNozzleLevelTemp ( NOZZLE_MINIUM );

                    Motor_State ( (U8) TYPE_MOTOR_BIDET, NOZZLE_MINIUM, (U8) TYPE_BIT_MOTOR_BIDET );

                    Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* 노즐 변경 */
        case AUTO_TEST_BIDET_NOZZLE:
            if ( mu8IsSetMotorEndFlagBidet == TRUE )
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                
                if ( Factory.u16AutoTestTime != 0 )
                {
                    Factory.u16AutoTestTime--;
                }
                else
                {
                    Factory.u16AutoTestTime = AUTO_TEST_3SEC;
                    Factory.u8AutoTestStep++;
                }
            }
        break;

        /* Delay */
        case AUTO_TEST_BIDET_DELAY:
            if ( Factory.u16AutoTestTime != 0 )
            {
                Factory.u16AutoTestTime--;
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
            }
            else
            {
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );

                Factory.u16AutoTestTime = AUTO_TEST_1SEC;
                Factory.u8AutoTestStep++;
            
            }
        break;

        /* 밸브 OFF */
        case AUTO_TEST_BIDET_END:
            if ( Factory.u16AutoTestTime != 0 )
            {
                Factory.u16AutoTestTime--;
                SetAirPlusCmd ( FALSE );
                SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
            }
            else
            {
                Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
                Motor_State ( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );

                Factory.u8AutoTestStep++;
            }
        break;
        
        
        /* 건조 동작 */
        case AUTO_TEST_DRY_START:

            SetDryLevel ( 3 );
            SetHotLevel ( 0 );
            SetSeatHeaterLevel ( 0 );

            SetModeSel ( TYPE_DRY );
            SetDryStart ( TRUE );
            SetDryTime ( DRY_TEST_TIME ); //8sec


            Factory.u8AutoTestStep++;

        break;

        /* 건조 끝 */
        case AUTO_TEST_DRY_END:
            if ( mu8IsSetMotorEndFlagClean == TRUE
                && mu8IsSetMotorEndFlagBidet == TRUE
                && mu8IsSetMotorEndFlagPress == TRUE
                && mu8IsSetMotorEndFlagWide == TRUE 
                && GetDryTime() == 0 )
            {
                SetDryLevel ( 0 );

                Factory.u8AutoTestStep = AUTO_TEST_INIT;
                Factory.u8AutoTest = FALSE;
            }
        break;

        default:

        break;				
    }
}


void FactoryKeyCheck ( U32 mu32KeyVal )
{
    U32 mu32FactoryTest;    
    U32 mu32WaterlessWaterLevelPass;
    U32 mu32FactoryAutoTest;


    mu32FactoryTest = HAL_KEY_2;                        /* Power */     //2020-01-19 김승원K 수동모드 진입키 power
    mu32WaterlessWaterLevelPass = HAL_KEY_1;            /* nozzle clean */
    mu32FactoryAutoTest = HAL_KEY_14;                   /* Stop*/       //2020-01-19 김승원K 수동모드 진입키 stop

    
    if ( Factory.Key == TRUE )
    {
        if ( Factory.KeyCount != 0 )
        {
            Factory.KeyCount--;
        }
        else
        {
            /* 10msec 이내 정지키 입력시 수동 테스트 모드 */
            if( mu32KeyVal == mu32FactoryTest )      
            {
                Factory.u8Test = TRUE;
                BUZZER_STEP ( BUZZER_SET_ON );
            }

            /* 10msec 이내 정지키 입력시 자동 테스트 모드 */
            if ( mu32KeyVal == mu32FactoryAutoTest )
            {
                Factory.u8Test_Remote = TRUE;
                BUZZER_STEP ( BUZZER_SET_ON );
            }

            /* 노즐청소 입력 상태에서 전원 인가시 수위센서 Err 미송신 */
            if ( mu32KeyVal == mu32WaterlessWaterLevelPass )
            {
                SetWaterlessLevelPass ( TRUE );
                BUZZER_STEP ( BUZZER_SET_ON );
            }            
            Factory.Key = FALSE;
        }    
    }
}       

