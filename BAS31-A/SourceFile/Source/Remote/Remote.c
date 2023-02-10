#pragma interrupt INTP11 INT_Remote_IR

#include "hw.h"
#include "r_cg_intc.h"
#include "Remote.h"

#include "SteppingMotor.h"
#include "Buzzer.h"
#include "Power.h"
#include "ErrorCheck.h"

__interrupt static void INT_Remote_IR (void)
{
#if CONFIG_REMOTE
    INT_Remote();
#endif
}




#define INTP_Remote_Disable()  R_INTC11_Stop()
#define INTP_Remote_Enable()   R_INTC11_Start()


#define REMOTE_COMUNICATE_MAX_COUNT         625         /* 62.5msec */
#define REMOTE_PARING_MAX_COUNT             200         /* 100msec * 200 = 20sec */
#define REMOTE_RECIVE_DELAY                 6           /* 6msec */

#define REMOTE_REM_INITIAL                  0x80        /* bit shift */
#define REMOTE_BUFFER_INITIAL               0           /* bit shift */

#define FUNCTION_CODE_STOP              1
#define FUNCTION_CODE_CLEAN             2
#define FUNCTION_CODE_BIDET             3
#define FUNCTION_CODE_DRY               4
#define FUNCTION_CODE_STER              5
#define FUNCTION_CODE_NOZZLECLEAN       6
#define FUNCTION_CODE_POWER             7
#define FUNCTION_CODE_ID                10
#define FUNCTION_CODE_SERVICE_DRY       11
#define FUNCTION_CODE_POWER_SAVE        15


typedef struct _remote_
{
    U16 IRCount;            /* IR PORT SET되면 Count */
    U8 Start;

    U8 ReceiveFinish;
    U16 ReceiveDelay;         /* receive finish, disable interrupt delay time */
    
    U8 RemStep;             /* bit shift step */
    U8 RemPosition;         /* bit shift 연산 */

    U8 Buffer;
    U8 BufferBYTE_1;
    U8 BufferBYTE_2;
    U8 BufferBYTE_3;
    U8 BufferBYTE_4;

    /* 1st BYTE */
    U8 BufferCompanyCode;         /* 0xa */
    U8 BufferFunctionCode;        /* 0 ~ 15 */

    /* 2nd BYTE */
    U8 BufferPressCode;
    U8 BufferSeatTempCode;
    U8 BufferWaterTempCode;

    /* 3th BYTE */
    U8 BufferNozzleCode;
    U8 BufferStateCode;

    /* 4th BYTE */
    U8 BufferPowerSaveCode;

    U8 Paring_Start;        /* Paring Mode */
    U8 Paring_Finish;       /* Paring Finish */
    U16 Paring_Time;    
    
 
} Remote_T;

Remote_T remote;

void InitRemote ( void )
{
    remote.IRCount = 0;
    remote.Start = 0;

    remote.ReceiveFinish = 0;
    remote.ReceiveDelay = 0;

    remote.RemStep = 0;
    remote.RemPosition = REMOTE_REM_INITIAL;

    remote.Buffer = REMOTE_BUFFER_INITIAL;
    remote.BufferBYTE_1 = 0;
    remote.BufferBYTE_2 = 0;
    remote.BufferBYTE_3 = 0;
    remote.BufferBYTE_4 = 0;
    
    remote.BufferCompanyCode = 0;
    remote.BufferFunctionCode = 0;

    remote.BufferPressCode = 0;
    remote.BufferSeatTempCode = 0;
    remote.BufferWaterTempCode = 0;

    remote.BufferNozzleCode = 0;
    remote.BufferStateCode = 0 ;
    remote.BufferPowerSaveCode = 0;

    remote.Paring_Start = FALSE;
    remote.Paring_Finish = FALSE;
    remote.Paring_Time = 0;
}

void SetRemoteParingStart ( U8 mu8val )
{
    remote.Paring_Start = mu8val;
}

U8 GetRemoteParingStart ( void )
{
    return remote.Paring_Start;
}


static U8 RemoconCheck(void);
static void MoveFunction(unsigned char mu8FunctionMode);

//falling edge
/***********************************************************************************************************************
* Function Name: System_ini
* Description  : 
***********************************************************************************************************************/
void INT_Remote(void)
{
    U8 mu8buzzer;
    
	if(remote.IRCount >= 40 
        && remote.IRCount <= 50) //4~5msec
	{
		remote.Start = TRUE;
		remote.IRCount = 0;
        
		remote.RemPosition = REMOTE_REM_INITIAL;
		remote.RemStep = 1;

		remote.Buffer = REMOTE_BUFFER_INITIAL;
		return;
	}

	if( remote.Start == TRUE )
	{
	    /* bit 연산 */
		if ( remote.IRCount > 0 
            && remote.IRCount <= 7 ) // ~700usec
		{
            remote.Buffer = (U8)(remote.Buffer & (U8)(~remote.RemPosition));   //0
		}
		else if ( remote.IRCount >= 8 
            && remote.IRCount <= 20 )   //~2msec
		{
            remote.Buffer = (U8)(remote.Buffer | remote.RemPosition);   //1
		}
		else
	    {
			remote.Start = FALSE;
			remote.IRCount = 0;
            
			remote.Buffer= REMOTE_BUFFER_INITIAL;
			remote.ReceiveFinish = FALSE;
			return;
		}

        /* Rem Step */
		if(remote.RemStep == 8)   // total 24 puls
		{
			remote.BufferBYTE_1 = remote.Buffer;
			remote.Buffer = REMOTE_BUFFER_INITIAL;

            remote.RemPosition = REMOTE_REM_INITIAL;
			remote.RemStep++;
		}
		else if(remote.RemStep == 16)
		{
			remote.BufferBYTE_2 = remote.Buffer;
			remote.Buffer = REMOTE_BUFFER_INITIAL;

            remote.RemPosition = REMOTE_REM_INITIAL;
			remote.RemStep++;
		}
		else if(remote.RemStep == 24)
		{
			remote.BufferBYTE_3 = remote.Buffer;
			remote.Buffer = REMOTE_BUFFER_INITIAL;

            remote.RemPosition = REMOTE_REM_INITIAL;
			remote.RemStep++;
		}
		else if(remote.RemStep == 32)
		{
			remote.BufferBYTE_4 = remote.Buffer;
			remote.Buffer = REMOTE_BUFFER_INITIAL;
            
			remote.RemPosition = REMOTE_REM_INITIAL;
			remote.RemStep = REMOTE_BUFFER_INITIAL;

            remote.ReceiveFinish = TRUE;
			remote.Start = FALSE;

            if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
            || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
            || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
            || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
            {
                BUZZER_STEP ( BUZZER_WARNING );        /*..불가음..*/
            }
            else
            {
			    mu8buzzer = RemoconCheck();
                BUZZER_STEP ( mu8buzzer );
            }
		}		
		else
		{
			remote.RemPosition = (U8)(remote.RemPosition >> 1);
			remote.RemStep++;  //cntRin
		}
	}

	remote.IRCount = 0;
}


/***********************************************************************************************************************
* Function Name: System_ini
* Description  : 
***********************************************************************************************************************/
static U8 RemoconCheck(void)
{
    U8 mu8buzzer = BUZZER_NONE;
    
	unsigned char mu8Temp1 = 0;
	unsigned char mu8Temp2 = 0;
	unsigned char mu8Temp3 = 0;
	unsigned char mu8Temp4 = 0;
	unsigned char mu8Temp5 = 0;
	unsigned char mu8Temp6 = 0;
	unsigned char mu8Temp7 = 0;

	mu8Temp1 = remote.BufferBYTE_1;
	mu8Temp2 = remote.BufferBYTE_1 << 4;

	mu8Temp3 = remote.BufferBYTE_2;
	mu8Temp4 = remote.BufferBYTE_2 << 4;

	mu8Temp5 = remote.BufferBYTE_3;
	mu8Temp6 = remote.BufferBYTE_3 << 4;

	mu8Temp7 = remote.BufferBYTE_4;


    /* check bit */
	mu8Temp1 = mu8Temp1 ^ mu8Temp2 ^ mu8Temp3 ^ mu8Temp4 ^ mu8Temp5 ^ mu8Temp6 ^ mu8Temp7;  /*..check bit..*/
	mu8Temp1 &= 0b11110000;

    /* checksum */
	mu8Temp7 = mu8Temp7 << 4;
	mu8Temp7 &= 0b11110000;


    /*..정상 수신..*/
	if ( mu8Temp1 == mu8Temp7 )
	{
		remote.BufferCompanyCode  = remote.BufferBYTE_1 >> 4;                       /*..회사코드..*/
		remote.BufferFunctionCode = remote.BufferBYTE_1 & 0b00001111;               /*..동작 모드..*/

		remote.BufferPressCode      = ( remote.BufferBYTE_2 >> 4) & 0b00000111;  	/*.. 수압 Dial, 1~5..*/
        remote.BufferSeatTempCode   = ( remote.BufferBYTE_2 )     & 0b00000011;     /*..0(OFF),1,2,3..*/
        remote.BufferWaterTempCode  = ( remote.BufferBYTE_2 >> 2) & 0b00000011;		/*..온수/건조 온도 설정..0~3*/

		remote.BufferNozzleCode     = ( remote.BufferBYTE_3 >> 5) & 0b00000111;	    /*..노즐 설정 Dial 1~5..*/
        remote.BufferStateCode      = ( remote.BufferBYTE_3 >> 4) & 0b00000001;     /*..Mode..*/

		remote.BufferPowerSaveCode  = ( remote.BufferBYTE_4 >> 7) & 0b00000001;     /*..절전..*/

        if ( remote.BufferCompanyCode != 0x0A 
            || remote.BufferFunctionCode > 15 
            || remote.BufferPressCode > 6 
            || remote.BufferSeatTempCode > 3
            || remote.BufferWaterTempCode > 3
            || remote.BufferNozzleCode > 6
            || remote.BufferStateCode > 2
            || remote.BufferPowerSaveCode > 1
            || GetPowerOnOff() == FALSE )

		{
			return mu8buzzer;
		}

        SetWaterSupplyErrorReleaseKey ( TRUE );

/* Like에 맞는 상황 처리 */
#if 0
		if(F_AutoDrainStart == SET)
		{
	        u16AutoDrainTime1min = 0;
		    u16AutoDrainTime24hour = 0;	
            Stop_Function();		/*..확인필..*/
			mu8buzzer = BUZZER_SELECT;
			return mu8buzzer;
		}
		else{}

        /*..공장 테스트모드 진입후 리모컨 사용 불가..151221..*/
        if ( F_FactoryWaterDrain == SET 
            || F_SpecialSettingMode == SET 
            || F_Memento == SET 
			|| F_FactoryTestStart == SET 
			|| F_FactoryRemoteDisable == SET )
        {
            mu8buzzer = BUZZER_WARNING;
            return mu8buzzer;
        }
		else{}

        /* 급수 Err 제외한 Err 발생시 */
		if ( u8ControlErrors > 3 )
		{
			mu8buzzer = BUZZER_NONE;
			return mu8buzzer;
		}


/***********************************************************************************************************************/
        /* 수압 */
        if ( u8WaterPressureLevel != remote.BufferPressCode )
        {
            if ( remote.BufferPressCode == 0 )
            {
                u8WaterPressureLevel = 1;
                mu8buzzer = BUZZER_WARNING;
            }
			else if ( remote.BufferPressCode == 6 )
            {
                u8WaterPressureLevel = 5;
                mu8buzzer = BUZZER_WARNING;
            }
            else //if ( remote.BufferPressCode >= 1 && remote.BufferPressCode <= 5 )
            {
                u8WaterPressureLevel = remote.BufferPressCode;
                mu8buzzer = BUZZER_SELECT;
            }
        }
		else
		{}


/***********************************************************************************************************************/
		/*..변좌 온도 설정..*/
		

		if ( u8SeatTemperatureLevel != remote.BufferSeatTempCode)
		{
			u8SeatTemperatureLevel = remote.BufferSeatTempCode;
            mu8buzzer = BUZZER_SELECT;
		}
		else
		{
			/// Continue ///
		}
		
/***********************************************************************************************************************/
		/*..온수/건조 온도 설정..*/

        if ( ( u8Mode == TYPE_DRY
            && remote.BufferFunctionCode != FUNCTION_CODE_STOP 
            && remote.BufferFunctionCode != FUNCTION_CODE_CLEAN
			&& remote.BufferFunctionCode != FUNCTION_CODE_BIDET ) 
			|| remote.BufferFunctionCode == FUNCTION_CODE_DRY )/*..건조모드..*/
        {
			if(u8DryTempLevel != remote.BufferWaterTempCode)
			{
                u8DryTempLevel = remote.BufferWaterTempCode;
                mu8buzzer = BUZZER_SELECT;
			}
			else{}
        }
		else
		{
			if(u8WaterTemperatureLevel != remote.BufferWaterTempCode )
			{
                u8WaterTemperatureLevel = remote.BufferWaterTempCode;
                mu8buzzer = BUZZER_SELECT;
			}
			else{}
		}


/***********************************************************************************************************************/


        /*.. 노즐 설정..*/

        if ( u8NozzleLevel != remote.BufferNozzleCode )
        {
            if ( remote.BufferNozzleCode == 0 )
            {
                u8NozzleLevel = 1;
                mu8buzzer = BUZZER_WARNING;
            }
			else if ( remote.BufferNozzleCode == 6 )
            {
                u8NozzleLevel = 5;
                mu8buzzer = BUZZER_WARNING;
            }
            else //if ( remote.BufferNozzleCode >= 1 && remote.BufferNozzleCode <= 5 )
            {
                u8NozzleLevel = remote.BufferNozzleCode;
                mu8buzzer = BUZZER_SELECT;
            }
        }
		else
		{}
/***********************************************************************************************************************/
		/*..그린플러그 설정..*/


        if ( remote.BufferFunctionCode == FUNCTION_CODE_POWER_SAVE
            && remote.BufferPowerSaveCode != u8GreenPlugOn )
        {
            if(remote.BufferPowerSaveCode == 1)
            {
                u8GreenPlugOn = 1;
				mu8buzzer = BUZZER_GREENPLUG_ON;
            }
			else //if(remote.BufferPowerSaveCode == 0)
			{				
                u8GreenPlugOn = 0;
                mu8buzzer = BUZZER_GREENPLUG_OFF;
			}
            u32EcoSeatLevelUnDetectTime = 0;
        }
		else{}
/***********************************************************************************************************************/

        if( F_AgingMode == SET )
        {
            mu8buzzer = BUZZER_NONE;
            return mu8buzzer;
        }

      	MoveFunction(remote.BufferFunctionCode);        
#endif
		//////////////////////////////////////////////////////////
		return mu8buzzer;
	}
	else                                         /*..수신 에러..*/
	{
		return mu8buzzer;
	}

}


#if 0
/***********************************************************************************************************************
* Function Name: System_ini
* Description  : 
***********************************************************************************************************************/
static void MoveFunction(unsigned char mu8FunctionMode)
{
	switch(mu8FunctionMode)
	{
		/*..정지(Stop)..*/
		case 1:
			F1_Remocon_Stop = SET;
		break;

		/*..세정(Rear)..*/
		case 2:
			F2_Remocon_Clean = SET;
		break;

		/*..비데(Front)..*/
		case 3:
			F3_Remocon_Bidet = SET;
		break;

		/*..건조(Dry)..*/
		case 4:
			F4_Remocon_Dry = SET;
		break;

		/*..살균..*/
		case 5:
			F5_Remocon_Sterilization = SET;
		break;

		/*..노즐 세척..*/
		case 6:
			F6_Remocon_Nozzle_Clean = SET;
		break;

		/*..전원..*/
		case 7:
			F0_Remocon_Power_OFF = SET;
		break;

		/*..Air Plus..*/
		case 8:
			F_RemoAirPlus = SET;
		break;
		
		/*..어린이..*/
		case 9:
			F7_Remocon_Kid = SET;

			if(F_Child_Start != SET)
			{
			    u8DryTempLevel = 2;
			}
			else{}
			
		break;

		/*..ID 설정..*/
		case 10:

			if(remote.Paring_Start == SET)
			{
			    remote.Paring_Finish = SET;
			}
			else{}

		break;

		/*..Service Dry..*/
		case 11:

			if(F_NozzleClean_Start != SET)
			{
 	            if(F_EW_ALL_Start == SET)
  			    {
				     Motor_3Way_State(0);
			         Motor_4Way_State(0);
    			     EW_Stop();
                }
			    else{}

	            BUZZER_STEP(1);
	            F_DryService = 1;
                Dry_Start();
			}
			else
			{
                BUZZER_STEP(16);
			}
			
		break;			

		/*..그린플러그 설정..*/
		case 14:
			//greenplug
		break;

		default:
			NOP();
		break;
	}

	EcoPowerSaveMode_Stop();
}
#endif



void Evt_100usec_Remote_Handler ( void )
{
    /*..리모콘 Input..*/
	if ( GET_STATUS_REMOTE_POWER() == TRUE )
	{
	    if ( remote.IRCount < REMOTE_COMUNICATE_MAX_COUNT )
        {   
		    remote.IRCount++;
        }
        else
        {
            remote.IRCount = REMOTE_COMUNICATE_MAX_COUNT;
        }
	}
	else{}
}   



void Evt_1msec_Remote_Handler ( void )
{
    if ( remote.ReceiveFinish == TRUE )
	{
		INTP_Remote_Disable();

        if ( remote.ReceiveDelay < REMOTE_RECIVE_DELAY )
        {
            remote.ReceiveDelay++;
        }
        else
        {
            remote.ReceiveDelay = 0;
            remote.ReceiveFinish = FALSE;
        }
	}
    else
    {
		INTP_Remote_Enable();
        remote.ReceiveDelay = 0;
    }
}


void Evt_100msec_Remote_Pairing_Mode_Handler ( void )
        {
            if ( remote.Paring_Start == TRUE )
            {
                TURN_ON_REMOTE_POWER();
        
                /* Paring Finish condition */
                /* 1.Timer Over: 20sec */
                /* 2.ID Setting Finish */
                if ( remote.Paring_Time >= REMOTE_PARING_MAX_COUNT 
                    || remote.Paring_Finish == TRUE )
                {
                    remote.Paring_Start = FALSE;
                    remote.Paring_Finish = FALSE;
                    BUZZER_STEP ( BUZZER_SET_ON );       /*..특수모드 ON..*/
                }
                else
                {
                    remote.Paring_Time++;
                }
            }
            else
            {
                remote.Paring_Time = 0;
                remote.Paring_Finish = FALSE;
                TURN_OFF_REMOTE_POWER();
            }
        }

