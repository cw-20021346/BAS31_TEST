#include "Hw.h"
#include "Mode.h"
#include "Adc.h"
#include "Sterilize.h"

#include "SteppingMotor.h"
#include "Seat.h"
#include "eeprom.h"
#include "Setting.h"
#include "Buzzer.h"
#include "Valve.h"

#include "FactoryMode.h"

#include "Power.h"
#include "Waterlevel.h"
#include "Aging.h"

#define  	AFTER_VALVE_ON      0
#define  	AFTER_EW_ON         1
#define  	AFTER_EW_OFF        2
#define  	AFTER_FINISH        3

#define  	Body_DELAY          0
#define  	Body_EW_ON          1

#define STERILIZE_FINISH_DISPLAY_BLINK_TIME          50      /* 5sec */
#define STERILIZE_FINISH_DISPLAY_SEAT_TIME           30      /* 3sec */

#define  	STER_ERR_CONDITION_TIME     3       /*300msec*/
#define  	STER_ERR_AD                 39      
#define  	STER_ERR_DETECT_TIME        5       /*500msec*/



typedef struct _sterilize_
{
    U8 OnOff;
    
    U8 Body;
    U8 After;
    U8 All;
    
    U8 BodyStep;
    U8 AfterStep;
    U8 AllStep;
    
    U8 Reservation;
    U8 AllCount;
    U8 CountEnable;

    /* Display */
    /* 살균완료시 5sec간 Mood LED 점멸 후 ON상태 유지 */
    /* 착좌 감지하면 (3sec) Mood LED OFF */
    U8 DisplayFinish;           /* 살균 완료시 SET, 착좌 후 3초 경과시 CLEAR */
    U16 DisplayFinishBlinkTime; /* 살균 완료시 5sec간 Blink Count */
    U16 DisplayFinishSeatTime;  /* DisplayFinish SET 이면 Mood LED ON 상태 유지 */
                                /* 3sec간 착좌감지 시 DisplayFinish CLEAR, Mood LED OFF */
    
    U8 SettingBody;
    U16 BodyOnTimeTarget;

    U16 OnTime;
    U16 OffTime;
    U16 Delay;
    U16 ErrorTime;

    U16 PotteryValveOnTime;     /* 도기 살균에서 밸브 On-Off 제어 On Time */
    U16 PotteryValveOffTime;    /* 도기 살균에서 밸브 On-Off 제어 Off Time */
    
} Sterilize_T;

Sterilize_T Ster;

static void CheckAutoSter( void );
static void CheckSterError( void );

static void SterBody ( void );
static void SterAfter ( void );
static void SterAll ( void );
static void SterAllDisplay ( void );


void InitSter ( void )
{
    Ster.OnOff = FALSE;
    
    Ster.Body = FALSE;
    Ster.After = FALSE;
    Ster.All = FALSE;
    
    Ster.BodyStep = 0;
    Ster.AfterStep = 0;
    Ster.AllStep = FLOW_PATH_INITIAL;
    
    Ster.AllCount = 0;
    Ster.CountEnable = FALSE;
    Ster.DisplayFinish = FALSE;
    Ster.DisplayFinishBlinkTime = 0;
    Ster.DisplayFinishSeatTime = 0;
    
    Ster.OnTime = 0;
    Ster.OffTime = 0;
    Ster.Delay = 0;
    Ster.BodyOnTimeTarget = 0;
    Ster.ErrorTime = 0;

    Ster.SettingBody = TRUE;
    Ster.Reservation = FALSE;
    
    Ster.PotteryValveOnTime = 0;
    Ster.PotteryValveOffTime = 0;
}

void SetSterOnOff ( U8 mu8Val )
{
    Ster.OnOff = mu8Val;
}

U8 GetSterOnOff ( void )
{
    return Ster.OnOff;
}


void SetSterBody ( U8 mu8Val )
{
    Ster.Body = mu8Val;
}

U8 GetSterBody ( void )
{
    return Ster.Body;
}

void SetSterAfter ( U8 mu8Val )
{
    Ster.After = mu8Val;
}

U8 GetSterAfter ( void )
{
    return Ster.After;
}

void SetSterAll ( U8 mu8Val )
{
    Ster.All = mu8Val;
}

U8 GetSterAll ( void )
{
    return Ster.All;
}

void SetSterAllStep ( U8 mu8Val )
{
    Ster.AllStep = mu8Val;
}

U8 GetSterAllStep ( void )
{
    return Ster.AllStep;
}


void SetSterAllCount ( U8 mu8Val )
{
    Ster.AllCount = mu8Val;
}

U8 GetSterAllCount ( void )
{
    return Ster.AllCount;
}

void SetSterCountEnable ( U8 mu8Val )
{
    Ster.CountEnable = mu8Val;
}

U8 GetSterCountEnable ( void )
{
    return Ster.CountEnable;
}

void SetSterSettingBody ( U8 mu8Val )
{
    Ster.SettingBody = mu8Val;
    SaveEepromId( EEP_ID_BODY_CLEAN );
}

U8 GetSterSettingBody ( void )
{
    return Ster.SettingBody;
}

void ToggleSterSettingBody ( void )
{
    if ( Ster.SettingBody == TRUE )
    {
        Ster.SettingBody = FALSE;
    }
    else
    {
        Ster.SettingBody = TRUE;
    }
    SetSettingTime(SETTING_TIME);
    SaveEepromId( EEP_ID_BODY_CLEAN );
}

void SetSterReservation ( U8 mu8Val )
{
    Ster.Reservation = mu8Val;
}

U8 GetSterReservation ( void )
{
    return Ster.Reservation;
}

U16 GetSterDisplayFinishBlinkTime ( void )
{
    return Ster.DisplayFinishBlinkTime;
}

U16 GetSterDisplayFinishSeatTime ( void )
{
    return Ster.DisplayFinishSeatTime;
}


void EW_Stop( void )
{
    Ster.Body = FALSE;
    Ster.After = FALSE;
    Ster.All = FALSE;
    
    Ster.BodyStep = 0;
    Ster.AfterStep = 0;
    Ster.AllStep = FLOW_PATH_INITIAL;
    
    Ster.OnTime = 0;
    Ster.OffTime = 0;
    Ster.Delay = 0;
    Ster.BodyOnTimeTarget = 0;
	
	Ster.OnOff = FALSE;   //EW OFF	
}

void SterAllStart( void )
{
    Ster.AllCount = 0;
    Ster.Reservation = FALSE;
    
    Ster.All = TRUE;
    if ( GetFactoryTest() == TRUE 
        || GetFactoryTest_Remote() == TRUE )
    {
        Ster.AllStep = POTTERY_INITIAL;
    }
    else
    {
        Ster.AllStep = FLOW_PATH_INITIAL;
    }
}

void SterBodyStart( U16 mu16Time )
{
    Ster.Body = TRUE;
    Ster.BodyStep = 0;
    Ster.BodyOnTimeTarget = mu16Time;
}



void Evt_100msec_Sterilize_Handler ( void )
{
    if ( GetPowerOnOff() == FALSE 
        || GetWaterLevel() == FALSE
        || GetFactoryStart() == TRUE )
    {
        EW_Stop();
        return;
    }
    
    if ( Ster.Delay != 0 
        && IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        Ster.Delay--;
    }
    
    /*Ster Count Check Function*/
    CheckAutoSter();
    
 	
    SterBody();
    SterAfter();
    SterAll();

    /* Ster All Display */
    SterAllDisplay();

    /*Ster Module Err Check*/
    CheckSterError();

    if ( Ster.OnOff == TRUE )
    {
        TURN_ON_STER();
    }
    else
    {
        TURN_OFF_STER();
    }
}

static void CheckAutoSter( void )
{
    if( GetModeSel() == TYPE_WAIT 
        && GetSeatLevel() == FALSE )
    {
        //EW Count
	    if( Ster.CountEnable == TRUE )
	    {
	        Ster.CountEnable = FALSE;
	        Ster.AllCount++;
	    }
    	
        if( Ster.AllCount >= STER_ALL_PERIOD 
            || Ster.Reservation == TRUE )
  	    {
  	        SterAllStart();
  	    }
  	}
}

static void CheckSterError( void )
{
    /////////////////////////////////////////////////////////////////////////////////////
  	//ADC Feedback 감지 저전압(Vin = 10.0V / Vfb = 0.38V / SAR 78) 기준   2014.08.07 정한영1급
  	//ADC Feedback 감지 저전압(Vin = 8.0V / Vfb = 0.304V / SAR 62) 기준   2014.10.02 정한영1급
  	//ADC Feedback 감지 저전압(Vin = 5.0V / Vfb = 0.19V / SAR 39) 기준   2014.12.22 정한영1급
  	//EW ON 전압 (Vin = 21.2~21.9V / Vfb = 0.80~0.83V / SAR 165~170)
  	//EW OFF 전압 (Vin = 0.9~2.1V / Vfb = 0.03~0.08V / SAR 7~16)
  	//EW ON 도달시간 약 5msec/프로그램: 100msec 시간 주면 진입 못 할 수도 있음 -> 300msec
  	//Currnet Limit 1A
  	//////////////////////////////////////////////////////////////////////////////////////

  	if ( Ster.OnOff == TRUE )
  	{
    	if ( ( Ster.OnTime > STER_ERR_CONDITION_TIME )
    	    && ( GetEWSensor() < STER_ERR_AD ) )
    	{
    	    //0.5초 간 Feedback 되는 전압이 정상이 아니면, EW STOP
    	    if ( Ster.ErrorTime < STER_ERR_DETECT_TIME )
    	    {
    	        Ster.ErrorTime++;
    	    }
    	    else
    	    {
    	        //2014-08-18 EW Feedback 이상전압 걸리더라도 logic 자체는 유지하고 EW만 OFF
    	        Ster.OnOff = FALSE;
    	    }
    	}
    	else
    	{
    	    Ster.ErrorTime = 0;
    	}
    }
    else
    {
        Ster.ErrorTime = 0;
    }
}

#define AFTER_STER_ON_TIME      40
#define SEAT_CLEAN_ON_TIME      40


static void SterAfter ( void )
{
    U16 mu16OnTime;

    /* 2018-07-20 김영표님 착좌감지 노즐세척시 6.5sec->9sec : 미적용 */
    if ( GetSeatCleanStart() == TRUE )
    {
        mu16OnTime = SEAT_CLEAN_ON_TIME;        /* 1sec + 6.5sec + 1.5sec = 9sec */
    }
    else
    {
        mu16OnTime = AFTER_STER_ON_TIME;        /* 1sec + 4sec + 1.5sec = 6.5sec */
    }

        
    if ( Ster.After == TRUE )
    {
        switch( Ster.AfterStep )
    	{
    	    /*Valve On*/
    	    case AFTER_VALVE_ON:
   	        
    	        SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
    	        Ster.Delay = 10; //1초
    	        Ster.AfterStep++;
    		break;
    		
    		/*EW ON 4sec*/
    		case AFTER_EW_ON:
    		    if( Ster.Delay == 0)
    		    {
    		        if( Ster.OnTime < mu16OnTime )
    		        {
        		        Ster.OnTime++;
        		        //2015-01-02 착좌감지노즐세척(전세척)시 EW OFF
        		        if ( GetSeatCleanStart() == FALSE )
        		        {
        	                Ster.OnOff = TRUE;
        			    }
    		        }
    		        else
    		        {
    		            Ster.OnTime = 0;
    			        Ster.AfterStep++;
    			    }
    			}
    		break;
    		
    		case AFTER_EW_OFF:
    		    if ( Ster.OffTime < 15 )
    		    {
    		        Ster.OffTime++;
    		        //2015-01-18 후세척 중 통합살균 시 조건 추가
    		        if( Ster.All == FALSE )
	                {
	                    Ster.OnOff = FALSE;
	                }
    		    }
    		    else
    		    {
	                Ster.OffTime = 0;
	                Ster.AfterStep++;
      			}
    		break;
    		
    		case AFTER_FINISH:

                SetSeatCleanStart ( FALSE );
                
		        if( Ster.All == FALSE )
				{
				    EW_Stop();
                }
            break;
    	}
    }
}


static void SterBody ( void )
{
    if ( Ster.Body == TRUE )
  	{
  	    switch( Ster.BodyStep )
    	{
    	    /*Delay 1sec*/
    	    case Body_DELAY:
    	        if ( Ster.OffTime < 10 )
    	        {
    	            Ster.OffTime++;
    	        }
    	        else
    	        {
    	            Ster.OffTime = 0;
        	        Ster.BodyStep++;
        	    }
    	    break;
    	    
    	    /*EW ON 2sec*/
    	    case Body_EW_ON:
    	        if ( Ster.OnTime < Ster.BodyOnTimeTarget )
    	        {
    	            Ster.OnTime++;
	                Ster.OnOff = TRUE;
    	        }
    	        else
    	        {
    	            Ster.OnTime = 0;
    	            EW_Stop();
    	        }
    	    break;
    	}
  	}
}

static void WaterPumpControl ( void );
static void SterPotteryValveControl ( void );

/*****************************************************************************************
 FUNCTION NAME : static void SterAll ( void )

 DESCRIPTION
 *  -. 통합 살균 Sequence 
 
 PARAMETERS: void
 
 RETURN VALUE: void

 NOTES:
 
*****************************************************************************************/
static void SterAll ( void )
{
    /* Water Pump ON/OFF */
    WaterPumpControl();
    
    /* 도기 살균 시 압력을 줄이기 위해 Valve On-Off 제어 */
    SterPotteryValveControl();
    
    //대기모드에서 자동/수동 유로 살균
    if( ( Ster.All == TRUE )
        && ( GetModeSel() == TYPE_WAIT ) )
    {
  	    //유로살균 Step
  	    switch( Ster.AllStep )
  	    {
  	        ////////////////////////////////////////////////////////////////
  	        // 유로살균
  	        // 세정1단 - 세정2단- 비데1단- 비데2단 
  	        //////////////////////////////////////////////////////////////////

            /*SelfClean(Clean)*/
            case FLOW_PATH_INITIAL:
                SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
                Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
                Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );
                Ster.AllStep++;        
            break;

            /*Valve ON*/
            case FLOW_PATH_START:
  	            if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON
                    Ster.Delay = 10;    //1sec
                    Ster.AllStep++;
                }
            break;

            /* 1sec 후 EW 2sec ON */
            case FLOW_PATH_SELF_CLEAN:
                if( Ster.Delay == 0 )
  	            {
                    if ( Ster.OnTime < 20 )
                    {
                        Ster.OnTime++;
                        Ster.OnOff = TRUE;
                    }
                    else
                    {
                        Ster.OnTime = 0;
                        /*2018-09-12 이현강님. 유로살균시 수압 증가-> 3way 이동 시  valve off 요청 */
                        Ster.OnOff = FALSE;
                        SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
                        
                        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_2, (U8) TYPE_BIT_MOTOR_PRESS );      //수압 2단
                        Ster.AllStep++;
                    }
                }
            break;

            /* 수압 2단 */
            case FLOW_PATH_MOTOR_MOVE:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
                {
                    /*2018-09-12 이현강님. 유로살균시 수압 증가-> 비데유로-세정유로 전환시 valve off 요청 */
                    Ster.OnOff = TRUE;
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON

                    Ster.Delay = 20;    //2sec
                    Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_CLEAN_1, (U8) TYPE_BIT_MOTOR_WIDE );       //세정 1단 
                    Ster.AllStep++;
                }
            break;

            case FLOW_PATH_CLEAN_1:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    if( Ster.Delay == 0 )
  	                {
  	                    Ster.Delay = 20;    //2sec
                        Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_CLEAN_2, (U8) TYPE_BIT_MOTOR_WIDE );       //세정 2단 
                        Ster.AllStep++;
                    }
                }
                
            break;
  	        
            case FLOW_PATH_CLEAN_2:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    if( Ster.Delay == 0 )
  	                {
  	                    Ster.Delay = 20;    //2sec
                        Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_BIDET_1, (U8) TYPE_BIT_MOTOR_WIDE );       //여성 세정 1단 

                        /*2018-09-06 송민수님. 유로살균시 수압 증가-> 세정유로-비데유로 전환시 valve off 요청 */
                        SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
                        Ster.OnOff = FALSE;
                        Ster.AllStep++;
                    }
                }
            break;

            case FLOW_PATH_BIDET_1:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
      	            /*2018-09-06 송민수님. 유로살균시 수압 증가-> 세정유로-비데유로 전환시 valve off 요청 */
  	                SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON
  	                Ster.OnOff = TRUE;
  	                
                    if( Ster.Delay == 0 )
  	                {
  	                    Ster.Delay = 20;    //2sec
                        Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_BIDET_2, (U8) TYPE_BIT_MOTOR_WIDE );       //여성 세정 2단 
                        Ster.AllStep++;
                    }
                }
            break;

            case FLOW_PATH_BIDET_2:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    if( Ster.Delay == 0 )
  	                {
  	                    Ster.OnOff = FALSE;
                        Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_BIDET_2, (U8) TYPE_BIT_MOTOR_WIDE );       //원점 
                        Ster.AllStep++;
                    }
                }
            break;            

            /*2018-09-12 이현강님. 유로살균시 수압 증가-> 비데유로-세정유로 전환시 valve off 요청 */
            case FLOW_PATH_ORIGIN_1:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
                    Ster.OnOff = FALSE;
                    Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_CLEAN_2, (U8) TYPE_BIT_MOTOR_WIDE );       //원점 
                    Ster.AllStep++;
                }
            break;  

            case FLOW_PATH_ORIGIN_2:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON
                    Ster.OnOff = FALSE;
                    Motor_State( (U8) TYPE_MOTOR_WIDE,  WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );       //원점 
                    Ster.AllStep++;
                }
            break;  
  
  	        /*3way origin*/
  	        case FLOW_PATH_ORIGIN_3:
  	            if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
  	                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
  	                Ster.AllStep++;
  	            }
  	        break;
  	        
  	        case FLOW_PATH_FINISH:
  	            if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
  	            {
  	                Ster.AllStep++;
  	            }

  	        ////////////////////////////////////////////////////////////////
  	        // 노즐살균
  	        // 자기세척에 놓고 7초간 EW ON / 7초간 OFF
  	        //////////////////////////////////////////////////////////////////
  	        
  	        //노즐살균: EW ON
  	        case NOZZLE_VALVE_ON:
                if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  	            {
  	                SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON
  	                Ster.Delay = 5;    //0.5sec
  	                Ster.AllStep++;
  	            }
  	        break;
  	        
            /*7sec*/
  	        case NOZZLE_EW_ON:
                if( Ster.Delay == 0 )
  	            {
      	            if ( Ster.OnTime < 70 )
      	            {
      	                Ster.OnTime++;
    	                Ster.OnOff = TRUE;
      	            }
      	            else
      	            {
      	                Ster.OnTime = 0;
      	                Ster.AllStep++;
      	            }
                }
  	        break;
  	        
  	        //노즐살균: 7초 후 EW OFF
  	        case NOZZLE_FINISH:
  	            if ( Ster.OffTime < 70 )
  	            {
  	                Ster.OffTime++;
  	                Ster.OnOff = FALSE;
  	            }
  	            else
  	            {
  	                Ster.OffTime = 0;
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO ); //밸브 OFF
                    Ster.AllStep++;
  	            }
  	        break;

            case POTTERY_INITIAL:
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_POTTERY, (U8) TYPE_BIT_MOTOR_PRESS );     /* 도기 */
                Ster.AllStep++;
            break;

            ////////////////////////////////////////////////////////////////
  	        // 도기 살균
  	        // 도기 위치로 이동하여 EW 10sec ON / 4sec OFF
  	        //////////////////////////////////////////////////////////////////
  	        case POTTERY_PUMP_ON:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
                {
                    //TURN_ON_WATER_PUMP();
                    Ster.Delay = 5;
                    Ster.AllStep++;
                }
            break;

            case POTTERY_VALVE_ON:
                if ( Ster.Delay == 0 )
                {
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO ); //밸브 ON
                    Ster.Delay = 10;
                    Ster.AllStep++;
                }
            break;

            case POTTERY_EW_ON:
                if ( Ster.Delay == 0 )
                {
                    if ( Ster.OnTime < 100 )
                    {
                        Ster.OnTime++;
                        Ster.OnOff = TRUE;
                    }
                    else
                    {
                        Ster.OnTime = 0;
                        Ster.AllStep++;
                    }
                }
            break;

            case POTTERY_EW_OFF:
  	            if ( Ster.OffTime < 40 )
  	            {
  	                Ster.OffTime++;
  	                Ster.OnOff = FALSE;
  	            }
  	            else
  	            {
  	                Ster.OffTime = 0;
                    	//TURN_OFF_WATER_PUMP();
                   	SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                    	Ster.AllStep++;
  	            }
  	        break;

            case POTTERY_VALVE_OFF:
                /*
                if ( GetValveOnOff() == FALSE 
                    && GetValveOffTime() == 0 )
                */
                if( Ster.Delay == 0 )
                {
                    Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );     /* 원점 */
                    Ster.AllStep++;
                }
            break;

            case POTTERY_FINISH:
                if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
                {
                    EW_Stop();
                    if ( GetFactoryTest() == FALSE
                        && GetAgingStart() == FALSE )
                    {
#if CONFIG_STER_COMPLETE_DISPLAY
                        Ster.DisplayFinish = TRUE;
                        Ster.DisplayFinishBlinkTime = STERILIZE_FINISH_DISPLAY_BLINK_TIME;
#else
                        Ster.DisplayFinish = FALSE;
                        Ster.DisplayFinishBlinkTime = 0;
#endif
                        
                    }
                    /* 2018-08-07 이정선님 살균 종료시 부저음 삭제 */
//                    BUZZER_STEP(BUZZER_SET_OFF);        //수동살균 종료음
                }

            break;
                
  	    }
    }          	            
}

static void WaterPumpControl ( void )
{
    if( ( Ster.All == TRUE )
        && ( GetModeSel() == TYPE_WAIT ) )
    {
        if ( ( Ster.AllStep == POTTERY_PUMP_ON 
            && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
            || ( Ster.AllStep == POTTERY_VALVE_ON )
            || ( Ster.AllStep == POTTERY_EW_ON ) 
            || ( Ster.AllStep == POTTERY_EW_OFF )
            || ( Ster.AllStep == POTTERY_VALVE_OFF ) )
        {
            TURN_ON_WATER_PUMP();
        }
        else
        {
            TURN_OFF_WATER_PUMP();
        } 
    }
    else
    {
        TURN_OFF_WATER_PUMP();
    }
}

#define STER_POTTERY_VALVE_ON_TIME      10
#define STER_POTTERY_VALVE_OFF_TIME     30

/*****************************************************************************************
 FUNCTION NAME : static void SterPotteryValveControl ( void )

 DESCRIPTION
 *  -. 통합 살균 Sequence 중 도기 살균 중 WaterValve 동작 Control
 
 PARAMETERS: void
 
 RETURN VALUE: void

 NOTES:
 *  -. 2020-04-28. 품질 김윤영C 도기 살균시 압력이 높아져 안전변 터지는 문제 개선: 밸브 0.8-3초 간격으로 ON-OFF
 *  -. 2020-05-08. 김정연 팀장. 도기 살균 압력을 감소시키기 위해 On/Off 제어는 허용하나 0.1초 단위는 지양. 2초 ON, 2초 OFF 적용
 
*****************************************************************************************/
static void SterPotteryValveControl ( void )
{
 
    if( ( Ster.All == TRUE )
        && ( GetModeSel() == TYPE_WAIT ) )
    {
        if ( Ster.AllStep == POTTERY_EW_ON 
            || Ster.AllStep == POTTERY_EW_OFF )
        {
            if ( Ster.Delay == 0 )
            {
                /* Valve Off 3sec */
                if ( Ster.PotteryValveOffTime < STER_POTTERY_VALVE_OFF_TIME )
                {
                    Ster.PotteryValveOnTime = 0;
                    Ster.PotteryValveOffTime++;
                    SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                }
                else
                {
                    /* valve on 0.5sec */
                    if ( Ster.PotteryValveOnTime < STER_POTTERY_VALVE_ON_TIME )
                    {
                        Ster.PotteryValveOnTime++;
                        SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    }
                    else
                    {
                        Ster.PotteryValveOffTime = 0;
                        Ster.PotteryValveOnTime = 0;
                    }
                }
            }
        }
        else 
        {
            Ster.PotteryValveOffTime = 0;
            Ster.PotteryValveOnTime = 0;
        }
    }
}



static void SterAllDisplay ( void )
{
    /* 5sec Blink Count */
    if ( Ster.DisplayFinishBlinkTime != 0 )
    {
        Ster.DisplayFinishBlinkTime--;
    }


    /* 3sec 착좌 감지시 LED OFF */
    if ( GET_STATUS_SEAT() == FALSE )
    {
        if ( Ster.DisplayFinishSeatTime != 0 )
        {
            Ster.DisplayFinishSeatTime--;
        }
        else
        {
            Ster.DisplayFinishSeatTime = 0;
            Ster.DisplayFinish = FALSE;
        }
    }
    else
    {
        if ( Ster.DisplayFinish == TRUE )
        {
            Ster.DisplayFinishSeatTime = STERILIZE_FINISH_DISPLAY_SEAT_TIME;
        }
        else
        {
            Ster.DisplayFinishSeatTime = 0;
        }
    }
}

