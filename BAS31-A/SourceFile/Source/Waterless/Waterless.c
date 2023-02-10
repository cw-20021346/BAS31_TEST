#pragma interrupt INTST0 r_uart0_interrupt_send
#pragma interrupt INTSR0 r_uart0_interrupt_receive

#include "Hw.h"

#include "Waterless.h"
#include "Adc.h"
#include "Eeprom.h"
#include "Steppingmotor.h"
#include "Power.h"
#include "Processerror.h"
#include "Mode.h"
#include "Seat.h"
#include "Waterlevel.h"
#include "Display.h"
#include "Nozzle.h"

#define AT_RS232_STX	0xac

#define W_START 		0x10
#define W_END 			0x11
#define G_VERSIOIN 		0x20
#define R_CLEAN			0x30
#define R_BEDIT			0x40
#define R_SOLVALVE		0xa4
#define S_SOLVALVE		0xa5
#define R_AIRPUMP		0xa0
#define S_AIRPUMP		0xa1
#define R_WATERPUMP		0xa2
#define S_WATERPUMP		0xa3

#define G_TH1			0xb1
#define G_TH2			0xb2

#define R_WATERHETER		0xa6
#define S_WATERHETER		0xa7
#define R_EEPROM		0xc0
#define R_LEDALLON		0xd0
#define R_LEDALLOFF		0xd1
#define R_LEDSTATENOW		0xd2

//#define R_GREENPLUGSET		0xd3
#define R_WATER_INITILA_SET		0xd3
#define R_SEATSENSORON		0xd4


#define UART_MODE_IDLE		0
#define UART_MODE_LENGTH	1
#define UART_MODE_SEL_NO	2
#define UART_MODE_PAYLOAD	3
#define UART_MODE_ERROR		4

#define BYTE_SYNC           1
#define BYTE_LENGTH         1
#define BYTE_STOP_CONDITION 1

#define ERR_CNT_OFFSET      0xa0

#define UART_TXD_LENGTH		12
#define UART_RXD_LENGTH		12
#define CMD_LENGTH		    12


U8	AT_gu8TxData[UART_TXD_LENGTH];
U8	AT_gu8RxData[UART_RXD_LENGTH];      /* 0:Idle, 1:Length, 2:Sel No., 3:PayLoad, ... , final:Err Cnt */
U8	RXD_CMD[CMD_LENGTH];

U8	AT_F_RxComplete;                /* UART RxD Complete flag */

U8	AT_gu8UARTStateMode;            /* UART RxD Step  Idle-length-SelNo.-PayLoad-Chekcsum */
U8	AT_gc8RxdErrCnt;                /* UART RxD Error Count */

U8	AT_gu8RxdCounter;               /* RxD Step++ */
U8	AT_gu8TxdCounter;               /* TxD Step++ */


U8 gu8WATERLESS;                        /* Waterless Inspection Start SET: WStart, CLEAR: WEnd */
U8	gu8WMode;                           /* Inspection Step++*/
U16 gu16HeaterOffCount;                 /* Heater Off Timer */

/* CMD flag */
bit	F_WSTART;                       /* Start send */
bit F_ZigStart;                     /* 공압 Start send */
bit	F_Pneumatic;                    /* 공압 Start send */
bit	F_VER;                          /* model version send */
bit	F_BEDIT;                        /* 4way bidet wide move send */
bit	F_AIRPUMP;                      /* Air Pump On 동작..후.. Off send */
bit	F_WATERPUMP;                    /* Water Pump On 동작..후.. Off send */
bit	F_SOLVALVE;                     /* Valve On 동작..후.. Off send */
bit	F_WATERHETER;                   /* Water Heater Max 2초간..On 동작..후.. Off send */
bit	F_TH1;                          /* in sensor data send */
bit	F_TH2;                          /* out sensor data send */
bit	F_WRTE2P;                       /* 0x1C Address에..0x87 Write */
bit	F_LedAllOn;                     /* All LED ON send */
bit	F_LedAllOff;                    /* not used */
bit	F_LedStateNow;                  /* LED 현재상태..표시.. send */
bit	F_WEND;

/* not used */
bit	F_CLEAN;
bit F_SeatSenor;
//bit F_GreenPlugSet;
bit F_WaterInitialSet;



U8 gu8SeatDetect;       /* EEPROM DATA Matching OK */
bit F_SeatOk;           /* Seat Detect OK */
U8 gu8TestSeatLevel;    /* F_SeatDetect&&F_SeatOk -> SET */
bit F_LEDAllon_Check;   /* Inspection 2 always ALL ON */
U16 u16SeatSensorcnt;   /* Seat Detect Timer */

bit F_SeatOpenError;    /* Seat Temp Short/Open Err 시..SET, CLEAR 없음..*/

U16  u16Pallet3Time;
U8   u8Pallet3Cnt;

typedef enum
{
    PALLET3_INITIAL,

    PALLET3_DRY_HEATER_START,
    PALLET3_DRY_GREEN_LED,
    PALLET3_DRY_RED_LED,
    PALLET3_NOZZLE_ORIGIN,

    PALLET3_END,
    
} Pallet3_T;



extern volatile uint8_t * gp_uart0_tx_address;        /* uart0 transmit buffer address */
extern volatile uint16_t  g_uart0_tx_count;           /* uart0 transmit data number */


__interrupt void    r_uart0_interrupt_send(void)
{
#if 1
    if (g_uart0_tx_count > 0U)
    {
        STMK0 = 1U;    /* disable INTST0 interrupt */
        TXD0 = *gp_uart0_tx_address;
        STMK0 = 0U;    /* enable INTST0 interrupt */

        gp_uart0_tx_address++;
        g_uart0_tx_count--;
    }
    else
    {
        //r_uart0_callback_sendend();
#if CONFIG_UART_REGISTER
        SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */
#endif
    }
#endif


#if 0
    // TxD 완료..
	if (AT_gu8TxdCounter >= AT_gu8TxData[1] + BYTE_SYNC + BYTE_LENGTH )
	{
		AT_gu8TxdCounter = 0;
	}
    // TxD 연속..데이터..저장..
	else
	{
		AT_gu8TxdCounter++;
        //STMK0 = 1U;    /* disable INTST0 interrupt */
		TXD0 = AT_gu8TxData[AT_gu8TxdCounter];
        //STMK0 = 0U;    /* Enable INTST0 interrupt */
	}
#endif
}

__interrupt void    r_uart0_interrupt_receive(void)
{ 
    volatile uint8_t err_type;

    U8 mu8RxdBufferData;        /* RxD Data Buffer */
    
    static U8 mu8RxdLength;            /* Length Data Buffer */
    static U8 mu8RxdSelNo;
    static U8 mu8RxdCheckSum;          /* Checksum Data Buffer */

    
    err_type = (uint8_t)(SSR01 & 0x0007U);
    SIR01 = (uint16_t)err_type;

    if (err_type != 0U)
    {
        //r_uart0_callback_error(err_type);
    }
    
 
	mu8RxdBufferData = RXD0;

	switch(AT_gu8UARTStateMode)
	{
		case UART_MODE_IDLE : 
			if(mu8RxdBufferData == AT_RS232_STX)        // STX check
			{
				AT_gu8RxdCounter = 0;
				AT_gu8UARTStateMode = UART_MODE_LENGTH;
				AT_gu8RxData[AT_gu8RxdCounter++] = mu8RxdBufferData;
				mu8RxdCheckSum = mu8RxdBufferData;
			}
			else 
			{
				AT_gu8RxdCounter = 0;
			}
		break;
		
		case UART_MODE_LENGTH :
			AT_gu8RxData[AT_gu8RxdCounter++] = mu8RxdBufferData;
			mu8RxdLength = mu8RxdBufferData + BYTE_SYNC + BYTE_LENGTH;
			mu8RxdCheckSum ^= mu8RxdBufferData;
			AT_gu8UARTStateMode = UART_MODE_SEL_NO;
		break;

		case 	UART_MODE_SEL_NO :
			AT_gu8RxData[AT_gu8RxdCounter++] = mu8RxdBufferData;
			mu8RxdSelNo = mu8RxdBufferData;
			mu8RxdCheckSum ^= mu8RxdBufferData;
			AT_gu8UARTStateMode = UART_MODE_PAYLOAD;
		break;
				
		
		case UART_MODE_PAYLOAD : 

            /* PayLoad Data 연속..저장 */
			if (AT_gu8RxdCounter < mu8RxdLength)
			{
				AT_gu8RxData[AT_gu8RxdCounter++] = mu8RxdBufferData;
				mu8RxdCheckSum ^= mu8RxdBufferData;
			}
            /* Finish */
			else if (AT_gu8RxdCounter == mu8RxdLength)
			{
				AT_gu8RxData[AT_gu8RxdCounter] = mu8RxdBufferData;
				
				if (AT_gu8RxData[AT_gu8RxdCounter] == mu8RxdCheckSum)
				{
					AT_F_RxComplete = SET;
					// AT_gc8RxdErrCnt+0xa0 = error 횟수 계산.. 0xa1 = error 1번 발생..
					AT_gu8RxData[++AT_gu8RxdCounter] = AT_gc8RxdErrCnt + ERR_CNT_OFFSET;
					AT_gc8RxdErrCnt = 0;
					AT_gu8RxdCounter = 0;
					AT_gu8UARTStateMode = UART_MODE_IDLE;	
				}
				else
				{
					AT_gu8RxdCounter = 0;
					AT_gu8UARTStateMode = UART_MODE_IDLE;
					AT_gc8RxdErrCnt++;
				}
			}
			else 
			{
				AT_gu8RxdCounter = 0;
				AT_gu8UARTStateMode = UART_MODE_IDLE;
				AT_gc8RxdErrCnt++;
			}
		break;
        
		default:
			AT_gu8RxdCounter = 0;
			AT_gc8RxdErrCnt++;
			AT_gu8UARTStateMode = UART_MODE_IDLE;
		break;
	}	
}















void SetWaterLessSeatReady ( U8 mu8val )
{
    gu8SeatDetect = mu8val;
}
U8 GetWaterLessSeatReady ( void )
{
    return gu8SeatDetect;
}

void SetWaterLessInspection2Start ( U8 mu8val )
{
    gu8TestSeatLevel = mu8val;
}
U8 GetWaterLessInspection2Start ( void )
{
    return gu8TestSeatLevel;
}

void SetWaterLessInspection1Start ( U8 mu8val )
{
    gu8WATERLESS = mu8val;
}
U8 GetWaterLessInspection1Start ( void )
{
    return gu8WATERLESS;
}



void Pallet3_Init(void)
{
//////////////////////////////////////////////////////////////////////////  
//////////////////////////////////비데 워터리스 Pallet #3용 EEPROM READ//////////////////////////////////////////// 
    gu8SeatDetect=0;


//////////////////////비데 워터리스 Pallet #3용 변수 초기화///////////////
	F_SeatOpenError = 0;
	F_SeatOk = 0;
  	F_LEDAllon_Check = 0;
	u16SeatSensorcnt = 0;
    
    gu8TestSeatLevel=0;
    u8Pallet3Cnt = PALLET3_INITIAL;
    u16Pallet3Time = 0;



/* Eeprom.h에서..처리..*/
/*
    EepromByteRead(EEP_ID_WATERLESS_TEST, &mu8Readbuffer );
	if(mu8Readbuffer == WATERLESS_EEP_CHECK )   //읽어온 데이터값이 정상. 읽어온 값 flag에 저장
	{
		gu8SeatDetect = 1;
	}           
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}





#define WATERLESS_LEVEL_DETECT_TIME     10      /* 1sec */

U16 gu16WaterLevelTime = WATERLESS_LEVEL_DETECT_TIME;      /* 수위미감지여부..판단..1sec Count */


void Evt_100msec_Waterless_Counter_Handler ( void )
{
    /* 수위미감지여부..판단..1sec Count */
    if ( gu16WaterLevelTime != 0 )
    {
        gu16WaterLevelTime--;
    }

    
    /* W Process Timer */
    if ( gu16HeaterOffCount != 0 )
    {
        gu16HeaterOffCount--;
    }


    //워터리스 Pallet #3 용 100ms Timer
    if (u16Pallet3Time > 0)                
    {
        u16Pallet3Time--;
    }


    /* EEPROM Matching OK */
    if ( gu8SeatDetect == TRUE )
    {
        if ( GET_STATUS_SEAT() == FALSE ) //착좌감지
        {
            if ( u16SeatSensorcnt < 5 )     //500msec
            {
                u16SeatSensorcnt++;
            }
            else
            {
                gu8SeatDetect = 0;
                F_SeatOk = 1;
                F_LEDAllon_Check = 1;
            }
        }
        else
        {
            u16SeatSensorcnt = 0;
        }
    }
}

static void Send_Data(void);
static void Ack_Data(void);
static void NAck_Data(void);
static void WaterErr(void);
static void SendVer(void);
static void CompleteAction(U8 Cmd);
static void SendTH(U8 Ch);
static void WHeaterTimeOver(void);
static void E2PTimeOver(void);
static void ZigStart(void);
static void PneumaticStart(void);


void UART2_RxDPROCESS(void)
{
	U8 CMD;          /* PayLoad First Data */

    /* RxD Finish하면..Start */
	if ( AT_F_RxComplete == CLR ) 
    {
        return;
    }

    AT_F_RxComplete = CLR;
	
	CMD = AT_gu8RxData[3];      /* PayLoad First Data */
	
	if (CMD == W_START)
	{
			gu8WATERLESS = SET;          /* Waterless inspection Start */

			F_WSTART = 1;
			gu8WMode = 13;              /* what? */


#if 1       //W_Process에서..처리..

            TURN_ON_WATER_LEVEL_TEST();     /* 수위..강제..감지..*/


            /* PLC 8sec 대기후에도..모터동작중이면..강제모터초기화..*/
			//F_First3WayMove = F_First4WayMove = F_FirstBidetMove = F_FirstCleanMove = 0;
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_CLEAN );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_BIDET );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_PRESS );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_WIDE );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_DRY );

			//u16MotorCleanCount = u16MotorBidetNozzleCount = 0;
			//u16Motor3WayCount = u16Motor4WayCount = 0;
            SetMotorCleanMoveCount(0);
            SetMotorBidetMoveCount(0);
            SetMotorPressMoveCount(0);
            SetMotorWideMoveCount(0);
            SetMotorDryMoveCount(0);
            
            SetMotorStep ( TYPE_MOTOR_CLEAN, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_BIDET, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_PRESS, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_WIDE, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_DRY, MOTOR_STOP );

			Ack_Data();

            /* 3way 5단.. 4way Wide 2단..*/
            /*
			u8Mode3Way = 5;
			u8Mode4Way = 2;

			u8ModeBidetNozzle = 3; u8ModeCleanNozzle = 5;
			u8WaterPressureLevel = 5;
			Motor_3Way_State(3);		// 수압 3단 
			Motor_4Way_State(2);		// 세정 와이드
			Motor_Bidet_Nozzle_State(3);
			Motor_Clean_Nozzle_State(5);
			*/

            Motor_State( TYPE_MOTOR_PRESS, PRESS_3, TYPE_BIT_MOTOR_PRESS);
            Motor_State( TYPE_MOTOR_WIDE, WIDE_CLEAN_2, TYPE_BIT_MOTOR_WIDE);
            Motor_State( TYPE_MOTOR_CLEAN, 5, TYPE_BIT_MOTOR_CLEAN);
            Motor_State( TYPE_MOTOR_BIDET, 3, TYPE_BIT_MOTOR_BIDET);
#endif
			return;
	}
	
	if ( gu8WATERLESS == CLR ) 
    {
        return;
    }
	

	switch (CMD)
	{

		case W_END : 	// 테스트 완료, 비데/세정 노즐 원점
			F_WEND = 1;
			Ack_Data();
		break;
		
		case G_VERSIOIN :
			F_VER = 1;
			Ack_Data();
		break;
		
		case R_CLEAN :
			F_CLEAN = 1;
			Ack_Data();
		break;
		
		case R_BEDIT : 
			F_BEDIT = 1;
			Ack_Data();
		break;
		
		case R_SOLVALVE :
			F_SOLVALVE = 1;
			gu8WMode = 1;
			Ack_Data();
		break;

		case S_SOLVALVE :
			F_SOLVALVE = 0;
			gu8WMode = 2;
			Ack_Data();
		break;
		
		case R_AIRPUMP :
			F_AIRPUMP = 1;
			gu8WMode = 3;
			Ack_Data();
		break;

		case S_AIRPUMP :
			F_AIRPUMP = 0;
			gu8WMode = 4;
			Ack_Data();
		break;

		case R_WATERPUMP :
			F_WATERPUMP = 1;
			gu8WMode = 5;
			Ack_Data();
		break;

		case S_WATERPUMP :
			F_WATERPUMP = 0;
			gu8WMode = 6;
			Ack_Data();
		break;

		case G_TH1 :
			F_TH1 = 1;
			Ack_Data();
		break;

		case G_TH2 :
			F_TH2 = 1;
			Ack_Data();
		break;

		case R_WATERHETER :
			F_WATERHETER = 1;
			gu8WMode = 7;
			Ack_Data();
		break;

		case S_WATERHETER :
			F_WATERHETER = 0;
			gu8WMode = 8;			
			Ack_Data();
		break;
		
		case R_EEPROM :
			F_WRTE2P = 1;
			gu8WMode = 10;
			Ack_Data();
		break;
		
		case R_LEDALLON :
			F_LedAllOn = 1;
			gu8WMode = 12;
			Ack_Data();
		break;
/*		
		case R_LEDALLOFF :
			F_LedAllOn = 0;
			F_LedAllOff = 1;
			F_LedStateNow = 0;
			Ack_Data();
		break;
*/
		case R_LEDSTATENOW :
			F_LedStateNow = 1;
			gu8WMode = 12;			
			Ack_Data();
		break;

		case R_SEATSENSORON :			
			F_SeatSenor = 1;
			gu8WMode = 15;
			Ack_Data();
		break;

		case R_WATER_INITILA_SET :
			//F_GreenPlugSet = 1;
            F_WaterInitialSet = 1;
			gu8WMode = 17;
			Ack_Data();
		break;

		default :	
			NAck_Data();	
		break;
	}

}


/* Waterless 검사시 수위센서 검사 PASS */
/* 노즐청소 입력상태에서 전원인가시 SET FactoryMode.c에서 처리 */
U8 gu8WaterLevelPass = FALSE;

U16 gu16WaterLevelCount = 0;    /* Waterless 수위 감지중 Noise에 의한 Err 방지 */
#define WATER_LEVEL_DETECT_COUNT        10        /* 10회 2019-08-26 안성회C */
void SetWaterlessLevelPass ( U8 mu8val )
{
    gu8WaterLevelPass = mu8val;
}
U8 GetWaterlessLevelPass ( void )
{
    return gu8WaterLevelPass;
}

void W_PROCESS(void)
{

    static U8	E2PErrCnt = 0;

    U8 mu8ReadBuff;         /* EEPROM TEST READ DATA BUFFER */

    U8 mu8GetStatusWaterLevel;
    U8 mu8WaterLevelPass;

    
	UART2_RxDPROCESS(); 

	if ( gu8WATERLESS == CLR ) 
    {
        return;
    }


    /* Power Off */
    //F_PowerOn = 0;
    SetPowerOnOff( FALSE );

    /* All Err Clear */
	//F_WaterSupplyError = F_WaterSupplyCriticalError = F_WaterLeakError = F_SeatTempOverError =
  	//F_SeatTempShortOpenError = F_OutTempOverError = F_WaterTempShortOpenError = 0;
    InitError();
    
#if CONFIG_UART_REGISTER
    if ( ( SOE0 & _0001_SAU_CH0_OUTPUT_ENABLE ) == _0001_SAU_CH0_OUTPUT_ENABLE )
    {
        return;
    }
#endif
        
        mu8WaterLevelPass = GetWaterlessLevelPass();
        
        /* 수위미감지일경우..*/
        mu8GetStatusWaterLevel = GET_STATUS_WATER_LEVEL();
        if ( gu16WaterLevelTime == 0)      /* 1sec delay */
        {        
            if ( mu8GetStatusWaterLevel == WATER_LEVEL_WATERLESS_ERR )
            {            
                if ( gu16WaterLevelCount < WATER_LEVEL_DETECT_COUNT )
                {
                    gu16WaterLevelCount++;
                }
                else
                {
                    if ( mu8WaterLevelPass == FALSE )
                    {         
                        WaterErr();
                    }
                    else
                    {}
                }
            }
            else
            {
                gu16WaterLevelCount = 0;
            }
        }
        else
        {}
            

        
    	// START : 0 Sec 
#if 0
        if ( F_WSTART )
        {
            TURN_ON_WATER_LEVEL_TEST();     /* 수위..강제..감지..*/
            
            
            /* PLC 8sec 대기후에도..모터동작중이면..강제모터초기화..*/
            //F_First3WayMove = F_First4WayMove = F_FirstBidetMove = F_FirstCleanMove = 0;
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_CLEAN );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_BIDET );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_PRESS );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_WIDE );
            ClearMotorFirstFlag ( TYPE_BIT_MOTOR_DRY );
            
            //u16MotorCleanCount = u16MotorBidetNozzleCount = 0;
            //u16Motor3WayCount = u16Motor4WayCount = 0;
            SetMotorCleanMoveCount(0);
            SetMotorBidetMoveCount(0);
            SetMotorPressMoveCount(0);
            SetMotorWideMoveCount(0);
            SetMotorDryMoveCount(0);
            
            SetMotorStep ( TYPE_MOTOR_CLEAN, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_BIDET, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_PRESS, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_WIDE, MOTOR_STOP );
            SetMotorStep ( TYPE_MOTOR_DRY, MOTOR_STOP );

            ClearMotorEndFlag ( TYPE_BIT_MOTOR_CLEAN );
            ClearMotorEndFlag ( TYPE_BIT_MOTOR_BIDET );
            ClearMotorEndFlag ( TYPE_BIT_MOTOR_PRESS );
            ClearMotorEndFlag ( TYPE_BIT_MOTOR_WIDE );
            ClearMotorEndFlag ( TYPE_BIT_MOTOR_DRY );
            
            Ack_Data();
            
            /* 3way 5단.. 4way Wide 2단..*/
            /*
            u8Mode3Way = 5;
            u8Mode4Way = 2;
            
            u8ModeBidetNozzle = 3; u8ModeCleanNozzle = 5;
            u8WaterPressureLevel = 5;
            Motor_3Way_State(3);        // 수압 3단 
            Motor_4Way_State(2);        // 세정 와이드
            Motor_Bidet_Nozzle_State(3);
            Motor_Clean_Nozzle_State(5);
            */
            
            Motor_State( TYPE_MOTOR_PRESS, PRESS_3, TYPE_BIT_MOTOR_PRESS);
            Motor_State( TYPE_MOTOR_WIDE, WIDE_CLEAN_2, TYPE_BIT_MOTOR_WIDE);
            Motor_State( TYPE_MOTOR_CLEAN, 5, TYPE_BIT_MOTOR_CLEAN);
            Motor_State( TYPE_MOTOR_BIDET, 3, TYPE_BIT_MOTOR_BIDET);

            F_WSTART = 0;
            F_ZigStart = 1;

            
        }

        if ( F_ZigStart )
        {
            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE ) // 공압 시작
            {
                F_Pneumatic = 1;

                ZigStart();
                F_ZigStart = 0;
            }
        }

        if (F_Pneumatic)
        {
            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE ) // 8 sec	공압 지그 체결
            {
                F_Pneumatic = 0;
                PneumaticStart();
            }
        }
#endif
#if 1
		if (F_WSTART)
		{
			if (F_Pneumatic)
			{
				if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                    && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )	// 공압 시작
				{
					F_WSTART = 0;
					F_Pneumatic = 0;
					PneumaticStart();
				}
			}
			else 
			{
			    if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE 
                    && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE )    // 8 sec	공압 지그 체결
				{
					ZigStart();
					F_Pneumatic = 1;
				}
			}
		}
#endif   
	// 8 Sec
		if (F_VER)
		{
			F_VER = 0;
			SendVer();
		}
		
	// 17 sec
/*
		if (F_RUNSOL)
		{
//			F_RUNSOL = 0;
			F_STOPSOL = 0;			
			UWaterValve = 0;        //밸브 ON
			CompleteAction(0xA4);			
		}
		if (F_STOPSOL)
		{
//			F_STOPSOL = 0;
			F_RUNSOL = 0;			
			UWaterValve = 1;        //밸브 Off
			CompleteAction(0xA5);			
		}	
*/
		if (F_SOLVALVE)
		{
		    TURN_ON_VALVE();
			if (gu8WMode == 1) 
			{
				CompleteAction(0xA4);
				gu8WMode = 0;
			}
		}
		else 
		{
			TURN_OFF_VALVE();
			if (gu8WMode == 2)
			{
				CompleteAction(0xA5);
				gu8WMode = 0;
			}
		}
		
		if (F_AIRPUMP)
		{
		    TURN_ON_AIR_PUMP();
			if (gu8WMode == 3) 
			{
				CompleteAction(0xA0);
				gu8WMode = 0;
			}
		}
		else
		{
			TURN_OFF_AIR_PUMP();
			if (gu8WMode == 4) 
			{
				CompleteAction(0xA1);
				gu8WMode = 0;
			}
		}
	
		if (F_WATERPUMP)
		{
			TURN_ON_WATER_PUMP();
			if (gu8WMode == 5) 
            {
				CompleteAction(0xA0);
				gu8WMode = 0;
			}
		}
		else
		{
			TURN_OFF_WATER_PUMP();
			if (gu8WMode == 6) 
			{
				CompleteAction(0xA1);
				gu8WMode = 0;
			}
		}
        
		if (F_TH1)
		{
			F_TH1 = 0;
			SendTH(1);
		}
        
		if (F_TH2)
		{
			F_TH2 = 0;
			SendTH(2);
		}
        
		if (F_WATERHETER)
		{
			TURN_ON_WATER_HEATER();
			if (gu8WMode == 7) 
			{
			    /*2018-11-15 WaterHeater Protect */
			    gu16HeaterOffCount = 20;            //2sec
				CompleteAction(0xA6);
				gu8WMode = 0;
			}
            /* 2sec 경과시 OFF */
			if ( gu16HeaterOffCount == 0)
			{
				WHeaterTimeOver();
				gu8WMode = 0;
				TURN_OFF_WATER_HEATER();
				F_WATERHETER = 0;
			}
		}
		else
		{
			TURN_OFF_WATER_HEATER();
			if (gu8WMode == 8)
			{
				CompleteAction(0xA7);
				gu8WMode = 0;
			}
		}
        
		if (F_WRTE2P)
		{
			if (gu8WMode == 10)
			{
			    EepromByteWrite ( EEP_ID_WATERLESS_TEST, WATERLESS_EEP_CHECK );
			    
				gu8WMode = 11;
				E2PErrCnt = 0;
			}
			else if (gu8WMode == 11)
			{
			    EepromByteRead ( EEP_ID_WATERLESS_TEST, &mu8ReadBuff );
				
				if (mu8ReadBuff == 0x87)
				{
					gu8WMode = 0;
					F_WRTE2P = 0;
					CompleteAction(0xc0);
				}
				else if (E2PErrCnt++ > 3)
				{
					gu8WMode = 0;
					F_WRTE2P = 0;
					E2PTimeOver();
				}
			}
		}

		if (F_SeatSenor)		// 착좌 설정.
		{
			if (gu8WMode == 15)
			{
			    EepromByteWrite ( EEP_ID_SEAT_DETECT, 0x01 );
				gu8WMode = 16;
				E2PErrCnt = 0;
			}
			else if (gu8WMode == 16)
			{
			    EepromByteRead(EEP_ID_SEAT_DETECT, &mu8ReadBuff );

				if (mu8ReadBuff == 0x01)
				{
					gu8WMode = 0;
					F_SeatSenor = 0;
					CompleteAction(0xc0);
                    SetSeatSetting ( TRUE );
				}
				else if (E2PErrCnt++ > 3)
				{
					gu8WMode = 0;
					F_SeatSenor = 0;
					E2PTimeOver();
				}
			}
		}

		//if (F_GreenPlugSet)
		if ( F_WaterInitialSet )
		{
			if (gu8WMode == 17)
			{
				EepromByteWrite ( EEP_ID_WATER_TANK_INITIAL, 0x01 );
				gu8WMode = 18;
				E2PErrCnt = 0;
			}
			else if (gu8WMode == 18)
			{
				EepromByteRead(EEP_ID_WATER_TANK_INITIAL, &mu8ReadBuff );
				if (mu8ReadBuff == 0x01)
				{
					gu8WMode = 0;
					//F_GreenPlugSet = 0;
                    F_WaterInitialSet = 0;
					CompleteAction(0xc0);
                    SetWaterLevelInitial( TRUE );
				}
				else if (E2PErrCnt++ > 3)
				{
					gu8WMode = 0;
					//F_GreenPlugSet = 0;
                    F_WaterInitialSet = 0;
					E2PTimeOver();
				}
			}			
		}




		if (F_LedAllOn)
		{
		    /*ProcessDisplayInspection1에서..처리..*/
		    //TurnOnAllLED();
		  	if (gu8WMode == 12)
		  	{
		  		gu8WMode = 0;
		  		CompleteAction(0xd0);
		  	}
		}
        
		if (F_LedStateNow)
		{
			
			F_LedAllOn = 0;
			F_LedStateNow = 0;
			CompleteAction(0xd2);
		}
	
	// 38 sec
		if (F_BEDIT)
		{
		    Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_BIDET_2, (U8) TYPE_BIT_MOTOR_WIDE );
			if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
			{
				F_BEDIT = 0;
				gu16HeaterOffCount = 0;
				CompleteAction(0x40);
			}
		}
            
		if (F_WEND)
		{

/*
            u16TargetMotorCleanNozzleCount = 0;
            u16TargetMotorBidetNozzleCount = 0;
            u16TargetMotor3WayCount = 0;
            u16TargetMotor4WayCount = 0;
            
            
            u8ModeBidetNozzle = 0; u8ModeCleanNozzle = 0;
            u8Mode3Way = u8Mode4Way = 0;
            
            
            F_First3WayMove = F_First4WayMove = F_FirstBidetMove = F_FirstCleanMove = 1;            
            
            u16MotorBidetNozzleCount = 0;
            u16Motor3WayCount = 0;  u16Motor4WayCount = 0;
            
            F_BidetNozzleEnd = F_CleanNozzleEnd = F_3WayEnd = F_4WayEnd = 0;
*/
            Motor_State( TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, TYPE_BIT_MOTOR_PRESS);
            Motor_State( TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, TYPE_BIT_MOTOR_WIDE);
            Motor_State( TYPE_MOTOR_CLEAN, 0, TYPE_BIT_MOTOR_CLEAN);
            Motor_State( TYPE_MOTOR_BIDET, 0, TYPE_BIT_MOTOR_BIDET);

            TURN_OFF_WATER_LEVEL_TEST();
			gu8WATERLESS = CLR;
			F_WEND = 0;
			CompleteAction(0x11);

		}
		


/*	
	
	if (giSys_Time > 300 && gu8WMode == 1) 
	{
		F_CLEAN = 1;
		gu8WMode = 0;
		giSys_Time = 0;
	}
	else if (gu8WMode == 1) 
	{
		//전체 LED ON
	  	ULED_Scan1 = ULED_Scan2	=	ULED_Scan3 = ULED_Scan4	=	0;      
	  	ULED_Select1 = ULED_Select2	=	ULED_Select3 = 1;
	}
	
	if (F_CLEAN)
	{
		Motor_4Way_State(5);      //비데 와이드
		if(F_4WayEnd)
		{
			F_CLEAN = 0;
			gu8WMode = 2;
		}
	}
	if (gu8WMode == 2)
	{
		if (giSys_Time > 200)
		{
			F_First3WayMove = F_First4WayMove = F_FirstBidetMove = F_FirstCleanMove = 1;
			u8ModeBidetNozzle = 0; u8ModeCleanNozzle = 0;
			u16MotorBidetNozzleCount = 0; u16MotorCleanCount = 0;
			u16Motor3WayCount = 0; 	u16Motor4WayCount = 0;
			u8Mode3Way = 0; u8Mode4Way = 0;
			u8Motor3WayState = 0;
			WATERLESS = 0;
			gu8WMode = 0;
		}
		else if (giSys_Time > 150)
		{
			UAirPump = 1;	// air pump off
		}
		else if (giSys_Time > 100)
		{
			UAirPump = 0;	// air pump on			
		}
	}
*/	
}

////////////////////////////////////워터리스 Pallet #3 Test Program  by.김진우2급//////////////////////////////////////
void Pallet_3(void)
{
/*
	if(u16Seat_Temperature == D_SeatError)     //120도이상 or -40도 이하
	{
		if(u8SeatTempShortOpenErrorTime_a++ < 200)        //2sec
		{
			
			F_SeatTempShortOpenErrorEnable = 1;
		}
		else
		{
			StopFunction();
			F_SeatTempShortOpenError = 1;
			F_SeatTempShortOpenErrorEnable = 0;
			F_PowerOn = 1;
		}
		//return;
	}

	
	AA = F_SeatTempShortOpenError;
*/	



	if ( IsErrorTypeStop() == TRUE )
	{
        return;
    }
	

	if ( ( F_SeatOk == TRUE ) 
        && ( gu8TestSeatLevel == FALSE ) )   //착좌감지 및 EEPROM READ값 OK 이면
	{
		gu8TestSeatLevel=1;                //착좌감지 flag High
		EepromByteWrite ( EEP_ID_WATERLESS_TEST, 0xFF );        //읽어온 EEPROM값 초기화
	}


    /* Inspection 2 Start */
	if ( gu8TestSeatLevel == TRUE )	
	{	
        /*초기동작..완료상태로..SETTING */
		//F_First3WayMove = F_First4WayMove = F_FirstBidetMove = F_FirstCleanMove = 0;
        ClearMotorFirstFlag ( TYPE_BIT_MOTOR_CLEAN );
        ClearMotorFirstFlag ( TYPE_BIT_MOTOR_BIDET );
        ClearMotorFirstFlag ( TYPE_BIT_MOTOR_PRESS );
        ClearMotorFirstFlag ( TYPE_BIT_MOTOR_WIDE );
        ClearMotorFirstFlag ( TYPE_BIT_MOTOR_DRY );
		


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		switch ( u8Pallet3Cnt )
		{	
			case PALLET3_INITIAL:

				u16Pallet3Time = 50;                            // 5초 Timer
				u8Pallet3Cnt = PALLET3_DRY_HEATER_START;
			break;
			
			case PALLET3_DRY_HEATER_START :
                if ( u16Pallet3Time != 0 )
                {
                    TURN_ON_FAN();
                    TURN_ON_DRY_RELAY();
                    TURN_ON_DRY_HEATER();

                    if(u16Pallet3Time <= 30)
    				{
    					F_LEDAllon_Check = 0;
    				}
                }
                else
                {
					TURN_OFF_DRY_RELAY();
                    TURN_OFF_DRY_HEATER();
					u16Pallet3Time = 50;                            // 5초 Timer
				    u8Pallet3Cnt = PALLET3_DRY_GREEN_LED;
                }               
			break;
				
			case PALLET3_DRY_GREEN_LED :
				if ( u16Pallet3Time != 0 )
				{
                    TURN_ON_SEAT_HEATER();

                    Nozzle_Move_Function ( MOVE_NARROW_FAST, 1, 5, TYPE_MOTOR_CLEAN, TYPE_BIT_MOTOR_CLEAN );
                    Nozzle_Move_Function ( MOVE_WIDE_SLOW, 1, 5, TYPE_MOTOR_BIDET, TYPE_BIT_MOTOR_BIDET );
                    
				}
				else
				{
                    TURN_OFF_SEAT_HEATER();

    				u16Pallet3Time = 50;                            // 5초 Timer
					u8Pallet3Cnt = PALLET3_DRY_RED_LED;
				}
			break;
			
			case PALLET3_DRY_RED_LED:
				if ( u16Pallet3Time != 0 )
				{
                    Nozzle_Move_Function ( MOVE_NARROW_FAST, 1, 5, TYPE_MOTOR_CLEAN, TYPE_BIT_MOTOR_CLEAN );
                    Nozzle_Move_Function ( MOVE_WIDE_SLOW, 1, 5, TYPE_MOTOR_BIDET, TYPE_BIT_MOTOR_BIDET );
				}
				else                             //10초 Timer 남은 시간 0초이면
				{
                    u16Pallet3Time = 30;                                // 3초 Timer
                    u8Pallet3Cnt = PALLET3_NOZZLE_ORIGIN;
				}
			break;
			
			case PALLET3_NOZZLE_ORIGIN :
                if ( u16Pallet3Time != 0 )
                {
                    Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
                    Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );

                    TURN_OFF_FAN();
                }
                else
				{
					u8Pallet3Cnt = PALLET3_END;
				}
			break;
			
			case PALLET3_END :
                EepromByteWrite(EEP_ID_WATERLESS_TEST, 0xFF );   //읽어온 EEPROM값 초기화
                //SetPowerOnOff( TRUE );
				F_SeatOk = 0;
                gu8TestSeatLevel = 0;
                u8Pallet3Cnt = PALLET3_INITIAL;
			break;
			
			default :
								
			break;
			
		}
	}
}

void ProcessDisplayInspection1 ( void )
{
    if ( F_LedAllOn == TRUE )
    {
        TurnOnAllLED();
    }
    else
    {
        TurnOffAllLED();
    }
        
}


void ProcessDisplayInspection2 ( void )
{
    TURN_OFF_LED_MOOD_BLUE();
    TURN_OFF_LED_MOOD_RED();

    if ( F_LEDAllon_Check == TRUE )
    {
        TurnOnAllLED();
    }
    else
    {
        if ( u8Pallet3Cnt == PALLET3_DRY_GREEN_LED )
        {
            HAL_SetBitDisplayBuffer( SEG_HOT_GREEN, ON );
            HAL_SetBitDisplayBuffer( SEG_SEAT_GREEN, ON );
            HAL_SetBitDisplayBuffer( SEG_HOT_RED, OFF );
            HAL_SetBitDisplayBuffer( SEG_SEAT_RED, OFF );
        }
        else if ( u8Pallet3Cnt == PALLET3_DRY_RED_LED )
        {
            HAL_SetBitDisplayBuffer( SEG_HOT_GREEN, OFF );
            HAL_SetBitDisplayBuffer( SEG_SEAT_GREEN, OFF );
            HAL_SetBitDisplayBuffer( SEG_HOT_RED, ON );
            HAL_SetBitDisplayBuffer( SEG_SEAT_RED, ON );
        }
        else
        {
            HAL_SetBitDisplayBuffer( SEG_HOT_GREEN, OFF );
            HAL_SetBitDisplayBuffer( SEG_SEAT_GREEN, OFF );
            HAL_SetBitDisplayBuffer( SEG_HOT_RED, OFF );
            HAL_SetBitDisplayBuffer( SEG_SEAT_RED, OFF );
        }
    }
}

static void Send_Data(void)
{
    U16 mu16num;
    
#if CONFIG_UART_REGISTER
    if ( ( SOE0 & _0001_SAU_CH0_OUTPUT_ENABLE ) == _0001_SAU_CH0_OUTPUT_ENABLE )
    {
        return;
    }
    STMK0 = 0U;    /* enable INTST0 interrupt */
    SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable UART0 output */  
#endif

    mu16num = AT_gu8TxData[1] + BYTE_SYNC + BYTE_LENGTH + BYTE_STOP_CONDITION;
    R_UART0_Send ( &AT_gu8TxData[0], mu16num ); 


//    AT_gu8TxdCounter = 0;
//    TXD0 = AT_RS232_STX;	

}

static void Ack_Data(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x04;
	AT_gu8TxData[2] = 0x01;
	AT_gu8TxData[3] = 'A';
	AT_gu8TxData[4] = 'C';
	AT_gu8TxData[5] = 'K';
	AT_gu8TxData[6] = 0x0;

	Send_Data();
	gu16HeaterOffCount = 0;	// 동작 시간 측정.
}

static void NAck_Data(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x04;
	AT_gu8TxData[2] = 0x01;
	AT_gu8TxData[3] = 'N';
	AT_gu8TxData[4] = 'A';
	AT_gu8TxData[5] = 'K';	
	AT_gu8TxData[6] = 0x0;

	Send_Data();
}

static void WaterErr(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x05;
	AT_gu8TxData[2] = 0x01;
	AT_gu8TxData[3] = 'W';
	AT_gu8TxData[4] = 'E';
	AT_gu8TxData[5] = 'R';
	AT_gu8TxData[6] = 'R';
	AT_gu8TxData[7] = 0x0;

	Send_Data();	
}

static void CompleteAction(U8 Cmd)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x08;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = Cmd;		// CMD CODE
	AT_gu8TxData[4] = 'A';		// CMD
	AT_gu8TxData[5] = 'C';
	AT_gu8TxData[6] = 'T';
	AT_gu8TxData[7] = ' ';
	AT_gu8TxData[8] = 'O';
	AT_gu8TxData[9] = 'K';
	AT_gu8TxData[10] = 0x0;		// End DATA
	
	Send_Data();
}

static void ZigStart(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x07;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = 'Z';		// CMD
	AT_gu8TxData[4] = 'I';
	AT_gu8TxData[5] = 'G';
	AT_gu8TxData[6] = 'S';
	AT_gu8TxData[7] = 'T';
	AT_gu8TxData[8] = 'R';
	AT_gu8TxData[9] = 0x0;		// End DATA
	
	Send_Data();
}

static void PneumaticStart(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x07;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = 'A';		// CMD
	AT_gu8TxData[4] = 'I';
	AT_gu8TxData[5] = 'R';
	AT_gu8TxData[6] = 'S';
	AT_gu8TxData[7] = 'T';
	AT_gu8TxData[8] = 'R';
	AT_gu8TxData[9] = 0x0;		// End DATA
	
	Send_Data();
}

static void SendVer(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x06;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = 'B';		// CMD
	AT_gu8TxData[4] = '3';
	AT_gu8TxData[5] = '1';
	AT_gu8TxData[6] = 'A';
	AT_gu8TxData[7] = '1';
	AT_gu8TxData[8] = 0x0;		// End DATA
	
	Send_Data();
}

static void WHeaterTimeOver(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x06;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = 'W';		// CMD
	AT_gu8TxData[4] = 'H';
	AT_gu8TxData[5] = 'T';
	AT_gu8TxData[6] = 'O';
	AT_gu8TxData[7] = 'V';
	AT_gu8TxData[8] = 0x0;		// End DATA
	
	Send_Data();
}

static void E2PTimeOver(void)
{
	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x06;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM
	AT_gu8TxData[3] = 'E';		// CMD
	AT_gu8TxData[4] = '2';
	AT_gu8TxData[5] = 'P';
	AT_gu8TxData[6] = 'O';
	AT_gu8TxData[7] = 'V';
	AT_gu8TxData[8] = 0x0;		// End DATA
	
	Send_Data();
}



static void SendTH(U8 Ch)
{
	U16 mu16Temp_Data;
	U16 Tmp_Temperauter;

    F32 mf32InSensor;
    F32 mf32OutSensor;

    mf32InSensor = GetInSensor();
    mf32OutSensor = GetOutSensor();
        
	if (Ch == 1)
	{
		Tmp_Temperauter = (U16)(mf32InSensor + 0.5);
		AT_gu8TxData[5] = '1';
	}
	if (Ch == 2) 
	{
		Tmp_Temperauter = (U16)(mf32OutSensor + 0.5);
		AT_gu8TxData[5] = '2';		
	}
	
//	mu16Temp_Data = HexToAsc(Tmp_Temperauter);

	if (Tmp_Temperauter > 9)
	{
		AT_gu8TxData[7] =  ( ( (U8) Tmp_Temperauter ) / 10 ) + 0x30;
		AT_gu8TxData[8] =  ( ( (U8) Tmp_Temperauter ) % 10 ) + 0x30;
	}
	else
	{
		AT_gu8TxData[7] = 0x30;
		AT_gu8TxData[8] = (U8)Tmp_Temperauter + 0x30;
	}

	AT_gu8TxData[0] = 0xac;
	AT_gu8TxData[1] = 0x07;		// length
	AT_gu8TxData[2] = 0x01;		// SEL NUM

	AT_gu8TxData[3] = 'T';
	AT_gu8TxData[4] = 'H';
	AT_gu8TxData[6] = ' ';

	AT_gu8TxData[9] = 0x0;		// End DATA

	Send_Data();	
}



