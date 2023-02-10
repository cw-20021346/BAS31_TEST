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

/* �ֱ� 249msec */
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
    U16 CyclePulse;          /* ����ũ�ν� ���� ���� Pulse ���� */    //gu8SetPulse

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

                        /* �Լ�����, �¼� */
const F32 Need_Watt_Compension_Pre[4][3] = {
/* �Լ��µ� 9�� �̸� */
    {   1.4f,  1.3f,  1.2f   },
/* �Լ��µ� 9�� ~ 14��  */
    {   1.5f,  1.3f,  1.2f   },
/* �Լ��µ� 14�� ~ 20��  */
    {   1.5f,  1.3f,  1.2f   },
/* �Լ��µ� 20���̻�  */
    {   1.5f,  1.3f,  1.3f   },
};
                        

static void Need_Watt_Nozzle ( U8 mu8SectionTempIN );

                            /* �Լ�����, �¼� */
const F32 Need_Watt_Compension_Nozzle[4][3] = {
/* �Լ��µ� 9�� �̸� */
    {   1.03f,  1.08f,  1.03f   },
/* �Լ��µ� 9�� ~ 14��  */
    {   1.03f,  1.08f,  1.03f   },
/* �Լ��µ� 14�� ~ 20��  */
    {   1.03f,  1.08f,  1.03f   },
/* �Լ��µ� 20���̻�  */
    {   0.95f,  1.08f,  1.02f   },
};
                 

static void Need_Watt_Start ( U8 mu8SectionTempIN );
                            /* �Լ�����, �¼� */
const F32 Need_Watt_Compension_Start[4][3] = {
/* �Լ��µ� 9�� �̸� */
    {   1.03f,  0.97f,  0.93f   },
/* �Լ��µ� 9�� ~ 14��  */
    {   1.03f,  0.97f,  0.93f   },
/* �Լ��µ� 14�� ~ 20��  */
    {   1.03f,  0.97f,  0.93f   },
/* �Լ��µ� 20���̻�  */
    {   1.03f,  0.97f,  0.93f   },
};
#endif

#if 0
                    /* �Լ�����, �¼�, ���� */
const F32 Need_Watt_Compension_Main[4][3][5] = {
/* �Լ��µ� 9�� �̸� */
{   {   1.02f,  1.02f,  0.95f,  0.95f,  0.95f   },
    {   0.96f,  0.96f,  0.96f,  0.96f,  0.96f   },
    {   0.97f,  0.92f,  0.92f,  0.92f,  0.92f   }, },

/* �Լ��µ� 9�� ~ 14��  */
{   {   1.10f,  1.00f,  1.00f,  1.00f,  1.00f   },
    {   0.96f,  0.96f,  0.96f,  0.96f,  0.96f   },
    {   0.97f,  0.92f,  0.92f,  0.92f,  0.92f   }, },

/* �Լ��µ� 14�� ~ 20��  */
{   {   0.95f,  0.95f,  0.95f,  0.95f,  0.95f   },
    {   1.02f,  1.02f,  0.93f,  0.93f,  0.93f   },
    {   0.97f,  0.97f,  0.92f,  0.92f,  0.92f   }, },

/* �Լ��µ� 20���̻�  */
{   {   0.95f,  0.95f,  0.95f,  0.95f,  0.95f   },
    {   1.02f,  1.02f,  0.93f,  0.93f,  0.93f   },
    {   0.98f,  0.98f,  0.98f,  0.98f,  0.98f   }, },

};


                    /* �Լ�����, �¼�, ���� */
const U16 Heater_Add_Compension[4][3][5] = {
/* �Լ��µ� 9�� �̸� */
{   {   100,    100,    200,    200,    600  },
    {   75,     100,    200,    600,    6001 },
    {   120,    100,    120,    120,    200  }, },

/* �Լ��µ� 9�� ~ 14��  */
{   {   120,    120,    200,    200,    600  },
    {   75,     100,    200,    600,    6001 },
    {   120,    100,    150,    200,    200  }, },

/* �Լ��µ� 14�� ~ 20��  */
{   {   120,    120,    120,    200,    600  },
    {   150,    200,    200,    600,    6001 },
    {   100,    200,    120,    6001,   6001 }, },

/* �Լ��µ� 20���̻�  */
{   {   6001,    6001,    6001,    6001,    6001  },
    {   6001,    6001,    6001,    6001,    6001  },
    {   6001,    6001,    6001,    6001,    6001  }, },
};
#endif




static void Need_Watt_Wait ( F32 mf32OutSensor, F32 mf32SetTemperaturePre );
/* �¼� Ʃ�� �Լ� */
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

/* 10�� ���� : ���¿����� �ް��ϰ� �÷��ش�.*/
const F32 Temperature_Gap_Compension_Over_2[TEMPERATURE_GAP_COMPENSION_MAXIUM+1] = {
/*GAP 0,        1,          2,          3,      4   */
    1.0f,     1.2f,       1.4f,       1.5f,     1.5f
};


const F32 Temperature_Gap_Compension_Under[TEMPERATURE_GAP_COMPENSION_MINIUM+1] = {
/*GAP 0,        1,          2,      3 */
    0.95f,     0.9f,       0.85f,     0.8f
};

/* 10�� ���� : ����ġ���� �µ� �ö󰡴� �� ����  */
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


    /* Heater �̵��� ���� */
    if ( GetOutSensor() > HOTWATER_TEMPERATURE_MAXIUM 
//        || GetValveHotCheck() == FALSE            /* valve�� ���� ���¿����� ���� (�����¼�) */
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
        /* ���� ���� ��� */
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
            /* Main ���� */
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
            /* ���� �̰��� && ���� ��� */
            if ( GetSeatLevel() == FALSE 
                && GetPowerSaveStart() == TRUE )
            {
                /* ���� ���� ��� */
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
        /* Main ���� */
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
            /* ���� �̰��� && ���� ��� */
            if ( GetSeatLevel() == FALSE 
                && GetPowerSaveStart() == TRUE )
            {
                /* ���� ���� ��� */
                FreezeProtect( mf32InSensor, mf32OutSensor);
            }
            else
            {
                Need_Watt_Wait ( mf32OutSensor, mf32SetTemperatureWait );
            }
        }
        else
        {
            /* ���� ���� ��� */
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
    /*����/�� PreHeating 300cc*/
    /******************************************************************/
    mf32Flow = FLOW_SELF_CLEAN;
    waterheater.NeedWatt=((mf32Flow)/60.0)*4.186*(mf32SetTemperature - mf32InSensor) ;
    waterheater.NeedWatt *= 1000;

    /*���Ϳ뷮�̳�..�῭��..�µ�����..������..�ʾ�..�������ô�..����X*/
    if ( GetBeforeStep() > STEP_BEFORE_SELF_DRAIN_VALVE_ON )
    {
        waterheater.NeedWatt = 0;
    }
    else
    {
        /* mf32SetTemperaturePre�� Before ���ۿ����� Target �µ� �������� ����  */
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
                    /* ���..ON.. 1.7sec */
                    waterheater.NeedWatt = waterheater.NeedWatt * 1.2f;
                }
                else
                {
                    /* ���..ON.. 2.1sec */
                    waterheater.NeedWatt = waterheater.NeedWatt * 1.2f;
                }
            }
            else //if ( GetModeSel() == TYPE_BIDET )
            {
                /* ���..ON.. 4.3sec */
                waterheater.NeedWatt = waterheater.NeedWatt * 0.8f;
            }

    //        waterheater.NeedWatt = waterheater.NeedWatt * 
    //        Need_Watt_Compension_Pre[mu8SectionTempIN][gu8SetWaterTemperature-1];  
        }
        /* ��ǥ�µ����� ���� ��� */
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

    /* �ɾ��..5������..����¹���..*/
    if ( GetModeSel() == TYPE_CLEAN
        && GetModeState() == STATE_CARE
        && mf32InSensor < 10.0f )
    {
        if ( ( GetWaterPressLevel() == 3 )
            && ( GetHotLevel() == 3 ) )
        {
            /* �¼�3��..����3����..����..*/
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
        /* ��ǥ�µ� ���� ���� ��� */
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
        /* ��ǥ�µ� ���� ���� ���(0��~) */
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
        /* ��ǥ�µ� ���� ��� */
        else
        {
            /* �������� */
        }
    }   



/******************************************************************************************************/
/* ���� �¼� �� ���� */
/******************************************************************************************************/

#if 0
    waterheater.NeedWatt = waterheater.NeedWatt * 
        Need_Watt_Compension_Main[mu8SectionTempIN][gu8SetWaterTemperature-1][gu8SetWaterDryStep-1];
#endif

//������ �̵� �� �µ��� Ʋ�����ٸ�..  
#if 0         
    /*�������(400cc) -> ���(300cc)�� ���а��ҷ� �µ� �ް��� ������ ����*/
    if ( waterheater.AddWatt_Main )
    {
        waterheater.AddWatt_Main--;
    }
    mf32AddWatt_Sum = 
        waterheater.AddWatt_Main/Heater_Add_Compension[mu8SectionTempIN][gu8SetWaterTemperature-1][gu8SetWaterDryStep-1];

    /*�ʱ� ��� �� �µ� �ϰ� ���� W*/ 
    waterheater.NeedWatt = waterheater.NeedWatt + (F32)mf32AddWatt_Sum;
#endif



    /* ���� �ʿ��� ��� ���� */
#if 0
    /*���к���*/
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
        // ���к����� ���� ���� ��Ʈ ���  
        waterheater.TotalWatt = HEATER_WATT_MAX ;          
        waterheater.CyclePulse  =  (U16)(waterheater.NeedWatt /HEATER_WATT)  ;  // ������ �޽� ����
        
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
    mu16TurnOn = mu16TurnOn/10;     /*8.3msec�� 83���� �����Ͽ� ������ 10 */

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
    /*����/�� ���� ���� 400cc*/
    /*��ǥ�µ� -1������ �ϼ�*/
    /******************************************************************/
    /* ���� �ʿ��� ��� ���� */
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
    /*����/�� 3way �̵� 300cc -> ��ǥ����*/
    /*��ǥ�µ� +2�� �ʰ����� ����*/
    /******************************************************************/
    /* ���� �ʿ��� ��� ���� */
#if 0
    waterheater.NeedWatt = waterheater.NeedWatt * 
            Need_Watt_Compension_Start[mu8SectionTempIN][gu8SetWaterTemperature-1];
#endif
//������ �̵� �� �µ��� Ʋ�����ٸ�..  
#if 0       
    /*������ �̵��� ���� W����*/
    if ( mu16MotorPressMoveCount > PRESS_COUNT_MIN 
        && mu16MotorPressMoveCount < PRESS_COUNT_MAX)
    {
        mf32FlowMoveAddWatt = (F32)PRESS_COUNT_MAX/mu16MotorPressMoveCount;

        if ( GetInSensor() < 14)
        {
            /*1��: 330 (90W) 5��: 1015(30W) */
            mf32FlowMoveAddWatt *= 30;
        }
        else// if(GetInSensor() >= 14 )
        {
            /*1��: 330 (150W) 5��: 1015(50W) */
            mf32FlowMoveAddWatt *= 50;
        }

        waterheater.NeedWatt += mf32FlowMoveAddWatt;
    }
#endif
}

