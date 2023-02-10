#include "hw.h"

#include "WaterHeater.h"

#include "Mode.h"
#include "Adc.h"
#include "Power.h"
#include "Powersave.h"
#include "Seat.h"
#include "Hotwater.h"
#include "ProcessError.h"
#include "SteppingMotor.h"
#include "WaterLevel.h"
#include "WaterPress.h"
#include "Sterilize.h"
#include "Valve.h"

#include "Factorymode.h"
#define HEATER_HALF_WAVE_TIME           83     /* 100usec, 10 * 83 = 8.3msec */

/* 주기 249msec */
#define HEATER_PERIOD                   249-1 /* 1msec, 8.3msec * 30ea = 249msec */

#define HEATER_WATT_MAX                 1500.0f
#define HEATER_WATT                     50.0f
#define CYCLE_PULSE_MAX                 30

#define HEATER_WATT_70                  HEATER_WATT * 21
#define HEATER_WATT_50                  HEATER_WATT * 15
#define HEATER_WATT_30                  HEATER_WATT * 9
#define HEATER_WATT_20                  HEATER_WATT * 6
#define HEATER_WATT_10                  HEATER_WATT * 3

typedef struct _waterheater_
{
    /* Period */
    U16 Period;             /* 1msec++, 8.3msec * 20ea = 166msec */

    /* Watt */
    F32 NeedWatt;            /* Need W */                           //gf32HeaterWatt
    F32 TotalWatt;          /* 1st heater MAX Watt */    
    U16 CyclePulse;          /* 제로크로싱 반파 제어 Pulse 객수 */    //gu8SetPulse

    U8 FreezeProtection;
} WaterHeater_T;

WaterHeater_T waterheater;



void InitWaterHeater ( void )
{
    waterheater.Period = 0;

    waterheater.NeedWatt = 0;
    waterheater.TotalWatt = HEATER_WATT_MAX;

    waterheater.CyclePulse = 0;

    waterheater.FreezeProtection = FALSE;
}

#if 0

                        /* 입수구간, 온수 */
const F32 Need_Watt_Compension_Pre[4][3] = {
/* 입수온도 9도 미만 */
    {   1.4f,  1.3f,  1.2f   },
/* 입수온도 9도 ~ 14도  */
    {   1.5f,  1.3f,  1.2f   },
/* 입수온도 14도 ~ 20도  */
    {   1.5f,  1.3f,  1.2f   },
/* 입수온도 20도이상  */
    {   1.5f,  1.3f,  1.3f   },
};
                        

static void Need_Watt_Nozzle ( U8 mu8SectionTempIN );

                            /* 입수구간, 온수 */
const F32 Need_Watt_Compension_Nozzle[4][3] = {
/* 입수온도 9도 미만 */
    {   1.03f,  1.08f,  1.03f   },
/* 입수온도 9도 ~ 14도  */
    {   1.03f,  1.08f,  1.03f   },
/* 입수온도 14도 ~ 20도  */
    {   1.03f,  1.08f,  1.03f   },
/* 입수온도 20도이상  */
    {   0.95f,  1.08f,  1.02f   },
};
                 

static void Need_Watt_Start ( U8 mu8SectionTempIN );
                            /* 입수구간, 온수 */
const F32 Need_Watt_Compension_Start[4][3] = {
/* 입수온도 9도 미만 */
    {   1.03f,  0.97f,  0.93f   },
/* 입수온도 9도 ~ 14도  */
    {   1.03f,  0.97f,  0.93f   },
/* 입수온도 14도 ~ 20도  */
    {   1.03f,  0.97f,  0.93f   },
/* 입수온도 20도이상  */
    {   1.03f,  0.97f,  0.93f   },
};
#endif

#if 0
                    /* 입수구간, 온수, 수압 */
const F32 Need_Watt_Compension_Main[4][3][5] = {
/* 입수온도 9도 미만 */
{   {   1.02f,  1.02f,  0.95f,  0.95f,  0.95f   },
    {   0.96f,  0.96f,  0.96f,  0.96f,  0.96f   },
    {   0.97f,  0.92f,  0.92f,  0.92f,  0.92f   }, },

/* 입수온도 9도 ~ 14도  */
{   {   1.10f,  1.00f,  1.00f,  1.00f,  1.00f   },
    {   0.96f,  0.96f,  0.96f,  0.96f,  0.96f   },
    {   0.97f,  0.92f,  0.92f,  0.92f,  0.92f   }, },

/* 입수온도 14도 ~ 20도  */
{   {   0.95f,  0.95f,  0.95f,  0.95f,  0.95f   },
    {   1.02f,  1.02f,  0.93f,  0.93f,  0.93f   },
    {   0.97f,  0.97f,  0.92f,  0.92f,  0.92f   }, },

/* 입수온도 20도이상  */
{   {   0.95f,  0.95f,  0.95f,  0.95f,  0.95f   },
    {   1.02f,  1.02f,  0.93f,  0.93f,  0.93f   },
    {   0.98f,  0.98f,  0.98f,  0.98f,  0.98f   }, },

};


                    /* 입수구간, 온수, 수압 */
const U16 Heater_Add_Compension[4][3][5] = {
/* 입수온도 9도 미만 */
{   {   100,    100,    200,    200,    600  },
    {   75,     100,    200,    600,    6001 },
    {   120,    100,    120,    120,    200  }, },

/* 입수온도 9도 ~ 14도  */
{   {   120,    120,    200,    200,    600  },
    {   75,     100,    200,    600,    6001 },
    {   120,    100,    150,    200,    200  }, },

/* 입수온도 14도 ~ 20도  */
{   {   120,    120,    120,    200,    600  },
    {   150,    200,    200,    600,    6001 },
    {   100,    200,    120,    6001,   6001 }, },

/* 입수온도 20도이상  */
{   {   6001,    6001,    6001,    6001,    6001  },
    {   6001,    6001,    6001,    6001,    6001  },
    {   6001,    6001,    6001,    6001,    6001  }, },
};
#endif




static void Need_Watt_Wait ( F32 mf32OutSensor, F32 mf32SetTemperaturePre );
/* 온수 튜닝 함수 */
static void Need_Watt_Pre ( F32 mf32InSensor, F32 mf32OutSensor, F32 mf32SetTemperature );
static void Need_Watt_Main ( F32 mf32OutSensor, F32 mf32OutSensor, F32 mf32SetTemperature );
static void Calculate_Watt ( void );
static void FreezeProtect ( F32 mf32InSensor, F32 mf32OutSensor );

#define TEMPERATURE_GAP_COMPENSION_MAXIUM       4
#define TEMPERATURE_GAP_COMPENSION_MINIUM       3

const F32 Temperature_Gap_Compension_Over[TEMPERATURE_GAP_COMPENSION_MAXIUM+1] = {
/*GAP 0,        1,          2,          3,      4   */
    1.0f,     1.2f,       1.4f,       1.5f,     1.5f
};

/* 10도 이하 : 저온에서는 급격하게 올려준다.*/
const F32 Temperature_Gap_Compension_Over_2[TEMPERATURE_GAP_COMPENSION_MAXIUM+1] = {
/*GAP 0,        1,          2,          3,      4   */
    1.0f,     1.2f,       1.4f,       1.5f,     1.5f
};


const F32 Temperature_Gap_Compension_Under[TEMPERATURE_GAP_COMPENSION_MINIUM+1] = {
/*GAP 0,        1,          2,      3 */
    0.95f,     0.9f,       0.85f,     0.8f
};

/* 10도 이하 : 연산치에서 온도 올라가는 것 방지  */
const F32 Temperature_Gap_Compension_Under_2[TEMPERATURE_GAP_COMPENSION_MINIUM+1] = {
/*GAP 0,        1,          2,      3 */
    0.9f,     0.8f,       0.7f,     0.6f
};

#define FLOW_SELF_CLEAN             0.72    //720cc

#define FLOW_CLEAN_NORMAL_1         0.38    //380cc
#define FLOW_CLEAN_NORMAL_2         0.46    //460cc
#define FLOW_CLEAN_NORMAL_3         0.54    //540cc

#define FLOW_CLEAN_CARE_1           0.3     //300cc
#define FLOW_CLEAN_CARE_2           0.38    //380cc
#define FLOW_CLEAN_CARE_3           0.46    //460cc


#define FLOW_BIDET_NORMAL_1         0.44     //440cc
#define FLOW_BIDET_NORMAL_2         0.52     //520cc
#define FLOW_BIDET_NORMAL_3         0.6      //600cc

#define FLOW_BIDET_CARE_1           0.38    //380cc
#define FLOW_BIDET_CARE_2           0.46    //460cc
#define FLOW_BIDET_CARE_3           0.54    //540cc





U8 gu8Test;

void Evt_10msec_WaterHeater_Handler ( void )
{

    F32 mf32InSensor;
    F32 mf32OutSensor;
    F32 mf32SetTemperature;
    F32 mf32SetTemperatureWait;

    U8 mu8GetFactoryAutoTest;
    U8 mu8GetFactoryAutoTestStep;
        

    mf32InSensor = GetInSensor();
    mf32OutSensor = GetOutSensor();
    mf32SetTemperature = GetHotTarget();
    mf32SetTemperatureWait = GetHotTargetWait();


    /* Heater 미동작 조건 */
    if ( GetOutSensor() > HOTWATER_TEMPERATURE_MAXIUM 
//        || GetValveHotCheck() == FALSE            /* valve가 열린 상태에서만 히팅 (순간온수) */
        || GetPowerOnOff() == FALSE 
        || GetWaterLevel() == FALSE
        || GetSterAll() == TRUE
        || IsErrorTypeStop() == TRUE )  
    {
        waterheater.NeedWatt = 0;
        waterheater.FreezeProtection = FALSE;
        Calculate_Watt();
        return;
    }

    mu8GetFactoryAutoTest = GetFactoryAutoTest();
    mu8GetFactoryAutoTestStep = GetFactoryAutoTestStep();
    /* Auto Test Heater Operation Section */
    if ( mu8GetFactoryAutoTest == TRUE )
    {
        if ( mu8GetFactoryAutoTestStep == AUTO_TEST_CLEAN_PRESS
            || mu8GetFactoryAutoTestStep == AUTO_TEST_CLEAN_WIDE
            || mu8GetFactoryAutoTestStep == AUTO_TEST_CLEAN_NOZZLE
            || mu8GetFactoryAutoTestStep == AUTO_TEST_BIDET_PRESS
            || mu8GetFactoryAutoTestStep == AUTO_TEST_BIDET_WIDE
            || mu8GetFactoryAutoTestStep == AUTO_TEST_BIDET_NOZZLE
            )
        {
            Need_Watt_Main ( mf32InSensor, mf32OutSensor, mf32SetTemperature );
        }
        else
        {
            waterheater.NeedWatt = 0;
        }
        
        Calculate_Watt();
        return;
    }
	/////////////////////////////////////////////////////////////////////////////////

    if ( GetHotLevel() == FALSE )
    {
        /* 동결 방지 기능 */
        if ( GetModeSel() == TYPE_WAIT )
        {       
            FreezeProtect( mf32InSensor, mf32OutSensor);
        }
        else
        {
            waterheater.NeedWatt = 0;
        }
    }
    else
    {
        if (  GetModeSel() == TYPE_CLEAN 
            || GetModeSel() == TYPE_BIDET )
        {
            if ( ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE 
                    || IsSetBidetMode( BIDET_MODE_BEFORE ) == TRUE )
                && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE 
                    || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
            {
                Need_Watt_Pre ( mf32InSensor, mf32OutSensor, mf32SetTemperature );
                gu8Test = 1;
            }
            /* Main 수류 */
            else if ( ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE 
                    || IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE )
                  && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE 
                    || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
            {
                Need_Watt_Main ( mf32InSensor, mf32OutSensor, mf32SetTemperature );
                gu8Test = 2;
            }
            else
            {
                waterheater.NeedWatt = 0;
                gu8Test = 3;
            }
        }
        else if ( GetModeSel() == TYPE_WAIT )
        {
            /* 착좌 미감지 && 절전 모드 */
            if ( GetSeatLevel() == FALSE 
                && GetPowerSaveStart() == TRUE )
            {
                /* 동결 방지 기능 */
                FreezeProtect( mf32InSensor, mf32OutSensor);
            }
            else
            {
                Need_Watt_Wait ( mf32OutSensor, mf32SetTemperatureWait );
            }
            gu8Test = 0;
        }
        else
        {
            waterheater.NeedWatt = 0;
            gu8Test = 0;
        }

    }
    Calculate_Watt();


#if 0
    if (  GetModeSel() == TYPE_CLEAN 
        || GetModeSel() == TYPE_BIDET )
	{
        if ( ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE 
                || IsSetBidetMode( BIDET_MODE_BEFORE ) == TRUE )
            && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE 
                || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
        {
            if ( GetHotLevel() == TRUE )
            {
                Need_Watt_Pre ( mf32InSensor, mf32OutSensor, mf32SetTemperature );
            }
            else
            {
                waterheater.NeedWatt = 0;
            }
            gu8Test = 1;
        }
        /* Main 수류 */
	    else if ( ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE 
                || IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE )
              && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE 
                || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
  		{
      		if ( GetHotLevel() == TRUE )
            {
                Need_Watt_Main ( mf32InSensor, mf32OutSensor, mf32SetTemperature );
            }
            else
            {
                waterheater.NeedWatt = 0;
            }
		    gu8Test = 2;
        }
        else
        {
            waterheater.NeedWatt = 0;
            gu8Test = 3;
        }
    }
    else if ( GetModeSel() == TYPE_WAIT )
    {
        if ( GetHotLevel() == TRUE )
        {
            /* 착좌 미감지 && 절전 모드 */
            if ( GetSeatLevel() == FALSE 
                && GetPowerSaveStart() == TRUE )
            {
                /* 동결 방지 기능 */
                FreezeProtect( mf32InSensor, mf32OutSensor);
            }
            else
            {
                Need_Watt_Wait ( mf32OutSensor, mf32SetTemperatureWait );
            }
        }
        else
        {
            /* 동결 방지 기능 */
            FreezeProtect( mf32InSensor, mf32OutSensor);
        }
        gu8Test = 0;
    }
    else
    {
    	waterheater.NeedWatt = 0;
        gu8Test = 0;
    }
    Calculate_Watt();

#endif
    


}

static void Need_Watt_Wait ( F32 mf32OutSensor, F32 mf32SetTemperatureWait )
{
    F32 mf32TempGap;

    mf32TempGap = mf32SetTemperatureWait - mf32OutSensor;

    if ( mf32TempGap > 10.0f )
    {
//#if CONFIG_PRE_HEATING_NOISE_REDUCE
//        waterheater.NeedWatt = HEATER_WATT_70;
//#else
        waterheater.NeedWatt = HEATER_WATT_MAX;
//#endif
    }
    else if ( mf32TempGap > 5.0f )
    {
        waterheater.NeedWatt = HEATER_WATT_50;
    }
    else if ( mf32TempGap > 0.0f )
    {
        waterheater.NeedWatt = HEATER_WATT_30;
    }
    else
    {
        waterheater.NeedWatt = 0;    /* 0% */
    }
}      

static void Need_Watt_Pre ( F32 mf32InSensor, F32 mf32OutSensor, F32 mf32SetTemperature )
{
    F32 mf32Flow;
    F32 mf32TempGap;

    /******************************************************************/
    /*세정/비데 PreHeating 300cc*/
    /******************************************************************/
    mf32Flow = FLOW_SELF_CLEAN;
    waterheater.NeedWatt=((mf32Flow)/60.0)*4.186*(mf32SetTemperature - mf32InSensor) ;
    waterheater.NeedWatt *= 1000;

    /*히터용량이나..잠열로..온도센서..반응이..늦어..노즐신장시는..히팅X*/
    if ( GetBeforeStep() > STEP_BEFORE_SELF_DRAIN_VALVE_ON )
    {
        waterheater.NeedWatt = 0;
    }
    else
    {
        /* mf32SetTemperaturePre는 Before 동작에서는 Target 온도 기준으로 제어  */
        mf32TempGap = mf32SetTemperature - mf32OutSensor;

        
        if ( mf32TempGap > 5.0f )
        {
            waterheater.NeedWatt = HEATER_WATT_MAX;
        }
        /* mf32SetTemperaturePre > mf32OutSensor */
        else if ( mf32TempGap > 0.0f )
        {
            if ( GetModeSel() == TYPE_CLEAN )
            {
                if ( GetModeState() == STATE_CARE )
                {
                    /* 밸브..ON.. 1.7sec */
                    waterheater.NeedWatt = waterheater.NeedWatt * 1.2f;
                }
                else
                {
                    /* 밸브..ON.. 2.1sec */
                    waterheater.NeedWatt = waterheater.NeedWatt * 1.2f;
                }
            }
            else //if ( GetModeSel() == TYPE_BIDET )
            {
                /* 밸브..ON.. 4.3sec */
                waterheater.NeedWatt = waterheater.NeedWatt * 0.8f;
            }

    //        waterheater.NeedWatt = waterheater.NeedWatt * 
    //        Need_Watt_Compension_Pre[mu8SectionTempIN][gu8SetWaterTemperature-1];  
        }
        /* 목표온도보다 높을 경우 */
        else
        {
            if ( mf32OutSensor > ( mf32SetTemperature + 3.0f ) )
            {
                waterheater.NeedWatt = 0;
            }
            else
            {
                if ( GetModeSel() == TYPE_CLEAN )
                {
                    NOP();
                }
                else //if ( GetModeSel() == TYPE_BIDET )
                {
                    //NOP();
                    waterheater.NeedWatt = waterheater.NeedWatt * 0.6f;
                }
            }
        }
    }
}



static void Need_Watt_Main ( F32 mf32InSensor, F32 mf32OutSensor, F32 mf32SetTemperature )
{
    F32 mf32Flow;
    F32 mf32TempGap;
    U16 mu16TempGap = 0;

    if ( GetModeState() == STATE_CARE )
    {
        if ( GetModeSel() == TYPE_CLEAN )
        {
            if ( GetWaterPressLevel() == 3 )
            {
                mf32Flow = FLOW_CLEAN_CARE_3;
            }
            else if ( GetWaterPressLevel() == 2 )
            {
                mf32Flow = FLOW_CLEAN_CARE_2;
            }
            else if ( GetWaterPressLevel() == 1 )
            {
                mf32Flow = FLOW_CLEAN_CARE_1;
            }
            else
            {
                mf32Flow = 0;
            }
        }
        else
        {
            if ( GetWaterPressLevel() == 3 )
            {
                mf32Flow = FLOW_BIDET_CARE_3;
            }
            else if ( GetWaterPressLevel() == 2 )
            {
                mf32Flow = FLOW_BIDET_CARE_2;
            }
            else if ( GetWaterPressLevel() == 1 )
            {
                mf32Flow = FLOW_BIDET_CARE_1;
            }
            else
            {
                mf32Flow = 0;
            }
        }
    }
    else
    {
        if ( GetModeSel() == TYPE_CLEAN )
        {
            if ( GetWaterPressLevel() == 3 )
            {
                mf32Flow = FLOW_CLEAN_NORMAL_3;
            }
            else if ( GetWaterPressLevel() == 2 )
            {
                mf32Flow = FLOW_CLEAN_NORMAL_2;
            }
            else if ( GetWaterPressLevel() == 1 )
            {
                mf32Flow = FLOW_CLEAN_NORMAL_1;
            }
            else
            {
                mf32Flow = 0;
            }
        }
        else
        {
            if ( GetWaterPressLevel() == 3 )
            {
                mf32Flow = FLOW_BIDET_NORMAL_3;
            }
            else if ( GetWaterPressLevel() == 2 )
            {
                mf32Flow = FLOW_BIDET_NORMAL_2;
            }
            else if ( GetWaterPressLevel() == 1 )
            {
                mf32Flow = FLOW_BIDET_NORMAL_1;
            }
            else
            {
                mf32Flow = 0;
            }
        }
    }

    waterheater.NeedWatt=((mf32Flow)/60.0)*4.186*(mf32SetTemperature - mf32InSensor) ;
    waterheater.NeedWatt *= 1000;

    /* 케어세정..5도에서..과상승방지..*/
    if ( GetModeSel() == TYPE_CLEAN
        && GetModeState() == STATE_CARE
        && mf32InSensor < 10.0f )
    {
        if ( ( GetWaterPressLevel() == 3 )
            && ( GetHotLevel() == 3 ) )
        {
            /* 온수3단..수압3단은..제외..*/
            NOP();
        }
        else
        {
            waterheater.NeedWatt = waterheater.NeedWatt * 0.9f;
        }
    }
    
    mf32TempGap = mf32SetTemperature - mf32OutSensor;

    if ( mf32TempGap > 5.0f )
    {
        waterheater.NeedWatt = HEATER_WATT_MAX;
    }
    else
    {
        /* 목표온도 보다 낮을 경우 */
        if ( mf32SetTemperature > mf32OutSensor )
        {
            mu16TempGap = (U16) ( mf32SetTemperature - mf32OutSensor );
            mu16TempGap = mu16TempGap + 1;
            if ( mu16TempGap > TEMPERATURE_GAP_COMPENSION_MAXIUM )
            {
                mu16TempGap = TEMPERATURE_GAP_COMPENSION_MAXIUM;
            }

            if ( mf32InSensor < 10.0f )
            {
                waterheater.NeedWatt = waterheater.NeedWatt * Temperature_Gap_Compension_Over[mu16TempGap];
            }
            else
            {
                waterheater.NeedWatt = waterheater.NeedWatt * Temperature_Gap_Compension_Over[mu16TempGap];
            }
        }
        /* 목표온도 보다 높을 경우(0도~) */
        else if ( mf32SetTemperature < mf32OutSensor ) 
        {
            mu16TempGap = (U16) ( mf32OutSensor - mf32SetTemperature );
            if ( mu16TempGap > TEMPERATURE_GAP_COMPENSION_MINIUM )
            {
                mu16TempGap = TEMPERATURE_GAP_COMPENSION_MINIUM;
            }

            if ( mf32OutSensor > 41.0f )
            {
                waterheater.NeedWatt = waterheater.NeedWatt * 0.3;
            }
            else
            {
                if ( mf32InSensor < 10.0f )
                {
                    waterheater.NeedWatt = waterheater.NeedWatt * Temperature_Gap_Compension_Under[mu16TempGap];
                }
                else
                {
                    waterheater.NeedWatt = waterheater.NeedWatt * Temperature_Gap_Compension_Under[mu16TempGap];
                }
            }
        }
        /* 목표온도 같을 경우 */
        else
        {
            /* 보정없음 */
        }
    }   



/******************************************************************************************************/
/* 순간 온수 모델 참고 */
/******************************************************************************************************/

#if 0
    waterheater.NeedWatt = waterheater.NeedWatt * 
        Need_Watt_Compension_Main[mu8SectionTempIN][gu8SetWaterTemperature-1][gu8SetWaterDryStep-1];
#endif

//유조변 이동 시 온도가 틀어진다면..  
#if 0         
    /*노즐신장(400cc) -> 토수(300cc)시 수압감소로 온도 급격히 떨어짐 보정*/
    if ( waterheater.AddWatt_Main )
    {
        waterheater.AddWatt_Main--;
    }
    mf32AddWatt_Sum = 
        waterheater.AddWatt_Main/Heater_Add_Compension[mu8SectionTempIN][gu8SetWaterTemperature-1][gu8SetWaterDryStep-1];

    /*초기 토수 시 온도 하강 보정 W*/ 
    waterheater.NeedWatt = waterheater.NeedWatt + (F32)mf32AddWatt_Sum;
#endif



    /* 보정 필요할 경우 적용 */
#if 0
    /*수압변동*/
    if ( GetResistorFlowUpDelay() != 0 )
    {
        waterheater.NeedWatt *= 1.06;
    }
      
    if ( GetResistorFlowDownDelay() != 0 )
    {
        waterheater.NeedWatt *= 0.96;
    }
        
    /* output water temperature*/
    if( GetOutSensor() > 41)
    {
        waterheater.NeedWatt *= 0.75;
    }
#endif
}



static void Calculate_Watt ( void )
{
    /* need W: waterheater.NeedWatt*/
    /* MAX W: waterheater.TotalWatt */
    /* Heater Pulse: waterheater.CyclePulse */
    /**************************************************************************/
    if ( waterheater.NeedWatt > 0 )
    {
        // 전압변동에 대한 히터 와트 계산  
        waterheater.TotalWatt = HEATER_WATT_MAX ;          
        waterheater.CyclePulse  =  (U16)(waterheater.NeedWatt /HEATER_WATT)  ;  // 내보낼 펄스 갯수
        
        if(waterheater.CyclePulse > CYCLE_PULSE_MAX)
        {
            waterheater.CyclePulse = CYCLE_PULSE_MAX;
        }
    }
    /*gf32HeaterWatt <= 0*/
    else     
    {
        waterheater.CyclePulse = 0;
    } 
}



static void FreezeProtect ( F32 mf32InSensor, F32 mf32OutSensor )
{
    if ( waterheater.FreezeProtection == FALSE )
    {
        if ( mf32OutSensor < 5.0f
            && mf32InSensor < 5.0f )
        {
            waterheater.NeedWatt = HEATER_WATT_10;        /* 10% */
            waterheater.FreezeProtection = TRUE;
        }
        else
        {
            waterheater.NeedWatt = 0;
            waterheater.FreezeProtection = FALSE;
        }
    }
    else
    {
        if ( mf32OutSensor > 6.0f
        && mf32InSensor > 6.0f )
        {
            waterheater.NeedWatt = 0;
            waterheater.FreezeProtection = FALSE;
        }
        else
        {
            waterheater.NeedWatt = HEATER_WATT_10;        /* 10% */
            waterheater.FreezeProtection = TRUE;
        }
    }
}



void Evt_1msec_WaterHeater_Handler ( void )
{
    if ( waterheater.Period < HEATER_PERIOD )
    {
        waterheater.Period++;
    }
    else
    {
        waterheater.Period = 0;
    }
}


void Evt_WaterHeater_Handler (void )
{
    U16 mu16TurnOn;

    mu16TurnOn = HEATER_HALF_WAVE_TIME * (U16)waterheater.CyclePulse;
    mu16TurnOn = mu16TurnOn/10;     /*8.3msec를 83으로 연산하여 나누기 10 */

    if ( mu16TurnOn > 0 )
#if 0
    if ( GetHotValveCheck() == TRUE 
        && GetPowerOnOff() == TRUE
        && GetWaterLevel() == TRUE
        && ( ( ( GetSeatSetting() == TRUE ) && ( GetSeatLevel() == TRUE ) )
            || ( GetSeatSetting() == FALSE ) ) )
#endif
    {
        if ( waterheater.Period < mu16TurnOn )
        {
            TURN_ON_WATER_HEATER();
        }
        else
        {
            TURN_OFF_WATER_HEATER();
        }
    }
    else
    {
        TURN_OFF_WATER_HEATER();
    }
}



 

static void Need_Watt_Nozzle ( U8 mu8SectionTempIN )
{
    /******************************************************************/
    /*세정/비데 노즐 신장 400cc*/
    /*목표온도 -1도까지 완성*/
    /******************************************************************/
    /* 보정 필요할 경우 적용 */
#if 0
    waterheater.NeedWatt = waterheater.NeedWatt * 
        Need_Watt_Compension_Nozzle[mu8SectionTempIN][gu8SetWaterTemperature-1];  
#endif
}



static void Need_Watt_Start ( U8 mu8SectionTempIN )
{
    U16 mu16MotorPressMoveCount;
    F32 mf32FlowMoveAddWatt;

    mu16MotorPressMoveCount = GetMotorPressMoveCount();

    /******************************************************************/
    /*세정/비데 3way 이동 300cc -> 목표유량*/
    /*목표온도 +2도 초과하지 말것*/
    /******************************************************************/
    /* 보정 필요할 경우 적용 */
#if 0
    waterheater.NeedWatt = waterheater.NeedWatt * 
            Need_Watt_Compension_Start[mu8SectionTempIN][gu8SetWaterTemperature-1];
#endif
//유조변 이동 시 온도가 틀어진다면..  
#if 0       
    /*유조변 이동에 따른 W보상*/
    if ( mu16MotorPressMoveCount > PRESS_COUNT_MIN 
        && mu16MotorPressMoveCount < PRESS_COUNT_MAX)
    {
        mf32FlowMoveAddWatt = (F32)PRESS_COUNT_MAX/mu16MotorPressMoveCount;

        if ( GetInSensor() < 14)
        {
            /*1단: 330 (90W) 5단: 1015(30W) */
            mf32FlowMoveAddWatt *= 30;
        }
        else// if(GetInSensor() >= 14 )
        {
            /*1단: 330 (150W) 5단: 1015(50W) */
            mf32FlowMoveAddWatt *= 50;
        }

        waterheater.NeedWatt += mf32FlowMoveAddWatt;
    }
#endif
}

