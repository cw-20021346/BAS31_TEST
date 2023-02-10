#include "hw.h"

#include "Mode.h"

#include "SteppingMotor.h"



#define MOTOR_DIRECT_FOWARD       1
#define MOTOR_DIRECT_BACK         0



#define MOTOR_ORIGIN_DELAY_100MSEC        1000      //0.1msec 기준 100msec






typedef struct _steppingmotor_
{
 
    U8 State;           /*단수 level*/
    
    U8 Direct;
    U8 OriginCheck;
    U8 ForwardToBack;
    U8 BackToForward;
    
    U16 MoveCount;      //u16Motor3WayCount
    U16 Target;
    U16 OriginDelay;
    U8 Step; //u8Motor3WayStep
    U8 StepBuffer;
    
} SteppingMotor_T;
SteppingMotor_T     StepMotor[TYPE_MAX];

U8 gu8First;           
U8 gu8End;          
U8 gu8Used;

U8 gu8InitReady = 0;


void InitSteppingMotor(void)
{
    U8 mu8type;
    
    for (mu8type=0 ; mu8type < TYPE_MAX ; mu8type++)
    {
        StepMotor[mu8type].State = 0;
        
        StepMotor[mu8type].Direct = 0;
        StepMotor[mu8type].OriginCheck = FALSE;
        StepMotor[mu8type].ForwardToBack = FALSE;
        StepMotor[mu8type].BackToForward = FALSE;
        
        StepMotor[mu8type].MoveCount = 0;
        StepMotor[mu8type].Target = 0;
        StepMotor[mu8type].OriginDelay = 0;
        StepMotor[mu8type].Step = MOTOR_STOP;
        StepMotor[mu8type].StepBuffer = 0;
    }
    
    gu8First = 0xFF;
    gu8End = 0xFF;
    gu8Used = 0;
}



void ClearMotorFirstFlag(U8 mu8Event )
{
    gu8First &= ~mu8Event;
}

U8 IsSetMotorFirstFlag( U8 mu8Event )
{
    if( (gu8First & mu8Event) ==  mu8Event )
    {
        return TRUE;
    }

    return FALSE;
}



/* 해당 비트만 0으로 */
void ClearMotorEndFlag(U8 mu8Event )
{
    gu8End &= ~mu8Event;
}
/* 해당 비트만 1로 */
void SetMotorEndFlag(U8 mu8Event )
{
    gu8End |= mu8Event;
}
/*해당 비트가 SET 되어 잇으면 TRUE */
U8 IsSetMotorEndFlag( U8 mu8Event )
{
    if( (gu8End & mu8Event) ==  mu8Event )
    {
        return TRUE;
    }

    return FALSE;
}



/* 해당 비트만 0으로 */
void ClearMotorUsedFlag(U8 mu8Event )
{
    gu8Used &= ~mu8Event;
}
/* 해당 비트만 1로 */
void SetMotorUsedFlag(U8 mu8Event )
{
    gu8Used |= mu8Event;
}
/*해당 비트가 SET 되어 잇으면 TRUE */
U8 IsSetMotorUsedFlag( U8 mu8Event )
{
    if( (gu8Used & mu8Event) ==  mu8Event )
    {
        return TRUE;
    }

    return FALSE;
}


void SetMotorStep ( U8 mu8Type, U8 mu8State )
{
    StepMotor[mu8Type].Step = mu8State;
}


void SetMotorCleanMoveCount( U16 mu16Val )
{
    StepMotor[TYPE_MOTOR_CLEAN].MoveCount = mu16Val;
}

U16 GetMotorCleanMoveCount( void )
{
    return StepMotor[TYPE_MOTOR_CLEAN].MoveCount;
}

void SetMotorBidetMoveCount( U16 mu16Val )
{
    StepMotor[TYPE_MOTOR_BIDET].MoveCount = mu16Val;
}

U16 GetMotorBidetMoveCount( void )
{
    return StepMotor[TYPE_MOTOR_BIDET].MoveCount;
}

void SetMotorWideMoveCount( U16 mu16Val )
{
    StepMotor[TYPE_MOTOR_WIDE].MoveCount = mu16Val;
}

U16 GetMotorWideMoveCount( void )
{
    return StepMotor[TYPE_MOTOR_WIDE].MoveCount;
}

void SetMotorDryMoveCount( U16 mu16Val )
{
    StepMotor[TYPE_MOTOR_DRY].MoveCount = mu16Val;
}

U16 GetMotorDryMoveCount( void )
{
    return StepMotor[TYPE_MOTOR_DRY].MoveCount;
}

void SetMotorPressMoveCount( U16 mu16Val )
{
    StepMotor[TYPE_MOTOR_PRESS].MoveCount = mu16Val;
}

U16 GetMotorPressMoveCount( void )
{
    return StepMotor[TYPE_MOTOR_PRESS].MoveCount;
}


void SetMotorPressState( U8 mu8val)
{
    StepMotor[TYPE_MOTOR_PRESS].State = mu8val;
}

U8 GetMotorPressState(void)
{
    return StepMotor[TYPE_MOTOR_PRESS].State;
}

typedef enum
{
    MOTOR_INITIAL,
    MOTOR_INITIAL_ADD_PULSE,
    MOTOR_INITIAL_ADD_PULSE_ACTIVE,
    FOWARD_TO_BACK_ADD_PULSE,       /* 순방향->역방향: 좀 더 뒤로 */
    BACK_TO_FOWARD_ADD_PULSE,       /* 역방향->순방향: 좀 더 앞으로 */
                                    /* 보정이 클수록 범위가 커짐 */

    MOTOR_PULSE_MAX,    
} MotorPulse_T;



#define PRESS_COUNT_MAX     1350
#define PRESS_COUNT_MIN     330

//23: 80분 뒤 5mm 뒤로 밀려남
//무부하의 경우 동일하게 주고, 3way는 물 압력 고려하여 불균형 
const U16 gu16InitialPulse[TYPE_MAX][MOTOR_PULSE_MAX] = 
{
    /*2018-07-24 이현강님 노즐 미복귀 개선: Init_add_active 추가 */
    /*Initial,  Init_add,   Init_add_active,    FtoB_add    BtoF_add*/
    {1400,       40,         40,                    19,         19      },     /*Clean*/
    {1500,       40,         40,                    23,         23      },     /*Bidet*/
    {2000,       60,         60,                    15,         15      },     /*Press*/
    {2000,       60,         60,                    15,         15      },     /*Wide*/
    {1500,       40,         60,                    23,         23      }      /*Dry*/
};

                    /* TARGET_STEP, Mode.Sel, hole */
const U16 gu16MotorStep[TARGET_STEP_MAX][TYPE_MODE_MAX][HOLE_MAX] = 
{
    /*TARGET_STEP_CLEAN*/
    {
    /*자기세척,  1단,    2단,   3단,    4단,    5단 */
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 1 (Clean)
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 2 (Bidet)
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 3 (Dry) 확인필요
    },

    /*TARGET_STEP_BIDET*/
    {
    /*자기세척,  1단,    2단,   3단,    4단,    5단 */
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 1 (Clean)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 2 (Bidet)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 3 (Dry) 확인필요
    },

    /*TARGET_STEP_PRESS*/
    {
/*일반세정 자기세척, 수압1(380cc),   수압2(460cc),   수압3(540cc),        도기,   EMPTY */
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 1 (Clean)
/*여성세정 자기세척, 수압1(440cc),   수압2(520cc),   수압3(600cc),        도기,   EMPTY */
            {0,         540,            870,            1170,             1800,    1800     },   //u8Mode == 2 (Bidet)
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 3 (Dry) 확인필요
            },

    /*TARGET_STEP_PRESS_CARE*/
    {
/*케어세정 자기세척, 수압1(300cc),   수압2(380cc),   수압3(460cc),      도기,   EMPTY */
            {0,         285,                470,        740,            1800,     1800     },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
            {0,         285,                450,        720,            1800,     1800     },   //u8Mode == 1 (Clean)
/*여성케어세정 자기세척, 수압1(380cc),   수압2(460cc),   수압3(540cc),        도기,   EMPTY */
            {0,         415,                530,        800,            1800,     1800     },   //u8Mode == 2 (Bidet)
            {0,         285,                470,        740,            1800,     1800     },   //u8Mode == 3 (Dry) 확인필요
            },

    /*TARGET_STEP_WIDE*/
    {
    /*Origin, CleanWide1, CleanWide1.5 CleanWide2, BidetWide2, BidetWide1,   EMPTY */
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 1 (Clean)
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 2 (Bidet)
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 3 (Dry) 확인필요
    },

    /*TARGET_STEP_DRY*/
    {
    /*자기세척,  1단,    2단,   3단,    4단,    5단 */
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 0 (대기), 자기세척, 수압2단, 도기세정 사용
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 1 (Clean)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 2 (Bidet)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 3 (Dry) 확인필요
    }

}; 


static void Motor_Initial_Function( U8 mu8Type, U16 mu16Time, U8 mu8TypeBit );
static void Motor_Move_Function( U8 mu8Type, U8 mu8TypeBit );

static void Motor_CW_Step( U8 mu8Type );
static void Motor_CCW_Step( U8 mu8Type );

static void Press_Motor_Port_Control( void );
static void Wide_Motor_Port_Control( void );
static void Clean_Motor_Port_Control( void );
static void Bidet_Motor_Port_Control( void );
static void Dry_Motor_Port_Control( void );

    


/*Timer*/
void Evt_Motor_MOVE_Function( U8 mu8Type, U16 mu16Time, U8 mu8TypeBit)
{
    if ( gu8InitReady >= 10 )       //초기시간 지연 후
    {
        Motor_Initial_Function( mu8Type, mu16Time, mu8TypeBit );
        Motor_Move_Function( mu8Type, mu8TypeBit );
        
        if ( mu8Type == TYPE_MOTOR_PRESS )
        {
            Press_Motor_Port_Control();
        }
        
        if ( mu8Type == TYPE_MOTOR_WIDE )
        {
            Wide_Motor_Port_Control();
        }
        
        if ( mu8Type == TYPE_MOTOR_CLEAN )
        {
            Clean_Motor_Port_Control();
        }
        
        if ( mu8Type == TYPE_MOTOR_BIDET )
        {
            Bidet_Motor_Port_Control();
        }

        if ( mu8Type == TYPE_MOTOR_DRY )
        {
            Dry_Motor_Port_Control();
        }
    }
}


void Evt_100msec_Motor_Initial ( void )
{
	if ( gu8InitReady < 10 )        // 1sec
	{
		gu8InitReady++;
	}
}




void Motor_State( U8 mu8Type, U8 mu83wayState, U8 mu8TypeBit )
{
#if 0
    //현재 상태와 관계없이 원점 입력시 60pulse 추가 이동함
    if( mu83wayState == 0 )		 //원점으로 이동할 경우	
    {
        StepMotor[mu8Type].OriginCheck = TRUE;
        ClearMotorUsedFlag(mu8TypeBit);
    }
#endif
    
    if( StepMotor[mu8Type].State != mu83wayState )     //3way 밸브 움직임 명령이 발생하면
    {
        if( mu83wayState == 0 )		 //원점으로 이동할 경우	
        {
            StepMotor[mu8Type].OriginCheck = TRUE;
            ClearMotorUsedFlag(mu8TypeBit);
        }

        SetMotorUsedFlag(mu8TypeBit);
        
        /*유량변경에 따른 히터 저항값 보정*/
        //if ( mu8Type == TYPE_MOTOR_PRESS )
        //{
            //Watt_Compensation_PressLevel( mu83wayState );
        //}
        
        
        
        //빽러쉬
        if ( StepMotor[mu8Type].Direct == MOTOR_DIRECT_FOWARD )    //기존에 순방향으로 이동했다면
        {
            if(StepMotor[mu8Type].State > mu83wayState)    //기존단수 > 입력 단수 -> 역방향
            {
                StepMotor[mu8Type].ForwardToBack = 1;        //기존 순방향이었다가 역방향으로 변경된 경우
            }
        }
        
        if ( StepMotor[mu8Type].Direct == MOTOR_DIRECT_BACK )     //기존에 역방향으로 이동했다면 
        {
            if(StepMotor[mu8Type].State < mu83wayState)      //기존단수 < 입력 단수 -> 순방향
            {
                StepMotor[mu8Type].BackToForward = 1;        //기존 역방향이었다가 순방향으로 변경된 경우
            }
        }
        
        if ( StepMotor[mu8Type].State < mu83wayState )    //기존 단수 < 입력 단수 -> 순방향
        {
            StepMotor[mu8Type].Direct = MOTOR_DIRECT_FOWARD;                //순방향
        }
        else                                  // 기존단수 >= 입력단수 ->역방향
        {
            StepMotor[mu8Type].Direct = MOTOR_DIRECT_BACK;                //역방향
        }
        
        StepMotor[mu8Type].State = mu83wayState;												//기존 단수에 입력 단수 대입
        ClearMotorEndFlag(mu8TypeBit);
    }
}












static void Motor_Initial_Function( U8 mu8Type, U16 mu16Time, U8 mu8TypeBit )
{
    U16 mu16OriginDelay = 0;
        
    if ( IsSetMotorFirstFlag(mu8TypeBit) == TRUE )
    {
        if ( StepMotor[mu8Type].MoveCount < gu16InitialPulse[mu8Type][MOTOR_INITIAL] )
    	{
    	    /* way motor 200pulse delay */
    	    if ( mu8Type == TYPE_MOTOR_PRESS 
                || mu8Type == TYPE_MOTOR_WIDE )
            {   
        		mu16OriginDelay = StepMotor[mu8Type].MoveCount%200;	//200pulse마다 delay
        		
        		if ( mu16OriginDelay == 0 )		
        		{
        			StepMotor[mu8Type].OriginDelay = MOTOR_ORIGIN_DELAY_100MSEC/mu16Time;
        			StepMotor[mu8Type].MoveCount++;
        		}
        		
        		if ( StepMotor[mu8Type].OriginDelay != 0 )
        		{
        			StepMotor[mu8Type].OriginDelay--;
        		}
        		else
        		{
        			StepMotor[mu8Type].MoveCount++;
        			Motor_CW_Step(mu8Type);
        		}
            }
            /* nozzle motor */
            else
            {
           		StepMotor[mu8Type].MoveCount++;
    			Motor_CW_Step(mu8Type);
            }
    	}
    	else
    	{
    	    ClearMotorFirstFlag(mu8TypeBit);
    		StepMotor[mu8Type].MoveCount	=	0;	
    		StepMotor[mu8Type].Step	=	MOTOR_STOP;	//정지
    	}
    }
}

static void Motor_Move_Function( U8 mu8Type, U8 mu8TypeBit )
{
    U8 mu8Mode;
    U8 mu8State;
    U8 mu8TargetStep;
    
    mu8Mode = GetModeSel();
    mu8State = GetModeState();

    /* Target Setting */
    if ( mu8Type == TYPE_MOTOR_CLEAN )
    {
        mu8TargetStep = TARGET_STEP_CLEAN;
    }
    else if ( mu8Type == TYPE_MOTOR_BIDET )
    {
        mu8TargetStep = TARGET_STEP_BIDET;
    }
    else if ( mu8Type == TYPE_MOTOR_PRESS )
    {
        if ( mu8State == STATE_CARE )
        {
            mu8TargetStep = TARGET_STEP_PRESS_CARE;
        }
        else
        {
            mu8TargetStep = TARGET_STEP_PRESS;
        }
    }
    else if ( mu8Type == TYPE_MOTOR_WIDE )
    {
        mu8TargetStep = TARGET_STEP_WIDE;
    }
    else //if ( mu8Type == TYPE_MOTOR_DRY )
    {
        mu8TargetStep = TARGET_STEP_DRY;
    }


    
	// 일반적인 상태에서
	if ( IsSetMotorFirstFlag(mu8TypeBit) == FALSE )
	{
	    /*원점으로 이동하는 경우*/
		if ( StepMotor[mu8Type].State == 0 )
		{
			if ( StepMotor[mu8Type].MoveCount != 0)
			{
				StepMotor[mu8Type].MoveCount--;
				Motor_CW_Step(mu8Type);
			}
			/*원점 이동 완료*/
			else
			{
				if(StepMotor[mu8Type].OriginCheck == TRUE )
				{
				    if ( GetModeState() == STATE_ACTIVE )
                    {
                        StepMotor[mu8Type].MoveCount    =   gu16InitialPulse[mu8Type][MOTOR_INITIAL_ADD_PULSE_ACTIVE];
                    }            
                    else
                    {
					    StepMotor[mu8Type].MoveCount	=	gu16InitialPulse[mu8Type][MOTOR_INITIAL_ADD_PULSE];
                    }
                    
					StepMotor[mu8Type].OriginCheck	=	FALSE;
				}
				else
				{
					StepMotor[mu8Type].Step	=	MOTOR_STOP;      //추가 이동 완료 후 모터 정지
					SetMotorEndFlag(mu8TypeBit);
				}
			}
		}
        else
        {
            /* Level 입력 시*/
            if ( StepMotor[mu8Type].State < HOLE_MAX )
            {
                StepMotor[mu8Type].Target = gu16MotorStep[mu8TargetStep][mu8Mode][StepMotor[mu8Type].State];

                
                if(StepMotor[mu8Type].MoveCount < StepMotor[mu8Type].Target)
                {
                    StepMotor[mu8Type].MoveCount++;
                    Motor_CCW_Step(mu8Type);
            
                }
                else if ( StepMotor[mu8Type].MoveCount > StepMotor[mu8Type].Target )
                {
                    StepMotor[mu8Type].MoveCount--;
                    Motor_CW_Step(mu8Type);
                }
                else                                                //Target에 도달했다면
                {
                    if(StepMotor[mu8Type].ForwardToBack == TRUE)  //기존 순방향이었다가 역방향으로 변경된 경우
                    {
                        StepMotor[mu8Type].ForwardToBack = FALSE;
                        StepMotor[mu8Type].MoveCount += gu16InitialPulse[mu8Type][FOWARD_TO_BACK_ADD_PULSE];
                    }
                    else if(StepMotor[mu8Type].BackToForward == TRUE)    //기존 역방향이었다가 순방향으로 변경된 경우
                    {
                        StepMotor[mu8Type].BackToForward = FALSE;
                        StepMotor[mu8Type].MoveCount -= gu16InitialPulse[mu8Type][BACK_TO_FOWARD_ADD_PULSE];
                    }
                    else
                    {
                        StepMotor[mu8Type].Step =   MOTOR_STOP; //정지
                        SetMotorEndFlag(mu8TypeBit);
            
                        /*목표 도달 시 저항 보상*/
                        //if ( mu8Type == TYPE_MOTOR_PRESS )
                        //{
                            //Resistor_Compensation_PressLevel();
                        //}
                    }
                }
            }   
            else
            {
                StepMotor[mu8Type].Step =   MOTOR_STOP; //정지
                SetMotorEndFlag(mu8TypeBit);
            }
        }
	}
}

/* 감소하는 방향 */
static void Motor_CW_Step( U8 mu8Type )
{
    /*정지였으면 이전 step부터 시작*/
    if ( StepMotor[mu8Type].Step == MOTOR_STOP)
    {
#if CONFIG_MOTOR_STEP
        if ( StepMotor[mu8Type].StepBuffer == 0)
        {
            StepMotor[mu8Type].Step = 3;
        }
        else
        {
            StepMotor[mu8Type].Step = StepMotor[mu8Type].StepBuffer-1;
        }
#else
        StepMotor[mu8Type].Step = StepMotor[mu8Type].StepBuffer;
#endif
    }
    
    if ( StepMotor[mu8Type].Step < 3 )
    {
        StepMotor[mu8Type].Step++;
    }
    else
    {
        StepMotor[mu8Type].Step = 0;
    }
    
    StepMotor[mu8Type].StepBuffer = StepMotor[mu8Type].Step;
}

/* 증가하는 방향 */
static void Motor_CCW_Step( U8 mu8Type )
{
    if ( StepMotor[mu8Type].Step == MOTOR_STOP )
    {
#if CONFIG_MOTOR_STEP
        if ( StepMotor[mu8Type].StepBuffer == 3)
        {
            StepMotor[mu8Type].Step = 0;
        }
        else
        {
            StepMotor[mu8Type].Step = StepMotor[mu8Type].StepBuffer+1;
        }
#else
        StepMotor[mu8Type].Step = StepMotor[mu8Type].StepBuffer;
#endif
    }
    
	if( StepMotor[mu8Type].Step == 0 )
	{
		StepMotor[mu8Type].Step	= 3;
	}	 
	else
	{
		StepMotor[mu8Type].Step--; 
	}
	
	StepMotor[mu8Type].StepBuffer = StepMotor[mu8Type].Step;
}

static void Press_Motor_Port_Control( void )
{
	switch( StepMotor[TYPE_MOTOR_PRESS].Step )
	{
		case 0:
			TURN_ON_PRESS_STEP_0();
			TURN_OFF_PRESS_STEP_1();
			TURN_OFF_PRESS_STEP_2();
			TURN_ON_PRESS_STEP_3();
		break;
					
		case 1:
			TURN_ON_PRESS_STEP_0();
			TURN_ON_PRESS_STEP_1();
			TURN_OFF_PRESS_STEP_2();
			TURN_OFF_PRESS_STEP_3();
		break;
		
		case 2:
			TURN_OFF_PRESS_STEP_0();
			TURN_ON_PRESS_STEP_1();
			TURN_ON_PRESS_STEP_2();
			TURN_OFF_PRESS_STEP_3();
		break;
			
		case 3:
			TURN_OFF_PRESS_STEP_0();
			TURN_OFF_PRESS_STEP_1();
			TURN_ON_PRESS_STEP_2();
			TURN_ON_PRESS_STEP_3();
		break;
				
		case MOTOR_STOP:
			TURN_OFF_PRESS_STEP_0();
			TURN_OFF_PRESS_STEP_1();
			TURN_OFF_PRESS_STEP_2();
			TURN_OFF_PRESS_STEP_3();
		break;
	}
}

static void Wide_Motor_Port_Control( void )
{
	switch( StepMotor[TYPE_MOTOR_WIDE].Step )
	{
		case 0:
			TURN_ON_WIDE_STEP_0();
			TURN_OFF_WIDE_STEP_1();
			TURN_OFF_WIDE_STEP_2();
			TURN_ON_WIDE_STEP_3();
		break;
					
		case 1:
			TURN_OFF_WIDE_STEP_0();
			TURN_OFF_WIDE_STEP_1();
			TURN_ON_WIDE_STEP_2();
			TURN_ON_WIDE_STEP_3();
		break;
		
		case 2:
			TURN_OFF_WIDE_STEP_0();
			TURN_ON_WIDE_STEP_1();
			TURN_ON_WIDE_STEP_2();
			TURN_OFF_WIDE_STEP_3();
		break;
			
		case 3:
            TURN_ON_WIDE_STEP_0();
            TURN_ON_WIDE_STEP_1();
            TURN_OFF_WIDE_STEP_2();
            TURN_OFF_WIDE_STEP_3();            
		break;
				
		case MOTOR_STOP:
			TURN_OFF_WIDE_STEP_0();
			TURN_OFF_WIDE_STEP_1();
			TURN_OFF_WIDE_STEP_2();
			TURN_OFF_WIDE_STEP_3();
		break;
	}
}

static void Clean_Motor_Port_Control( void )
{
	switch( StepMotor[TYPE_MOTOR_CLEAN].Step )
	{
		case 0:
			TURN_ON_CLEAN_STEP_0();
			TURN_OFF_CLEAN_STEP_1();
			TURN_OFF_CLEAN_STEP_2();
			TURN_ON_CLEAN_STEP_3();
		break;
					
		case 1:
			TURN_OFF_CLEAN_STEP_0();
			TURN_OFF_CLEAN_STEP_1();
			TURN_ON_CLEAN_STEP_2();
			TURN_ON_CLEAN_STEP_3();
		break;
		
		case 2:
			TURN_OFF_CLEAN_STEP_0();
			TURN_ON_CLEAN_STEP_1();
			TURN_ON_CLEAN_STEP_2();
			TURN_OFF_CLEAN_STEP_3();
		break;
			
		case 3:
			TURN_ON_CLEAN_STEP_0();
			TURN_ON_CLEAN_STEP_1();
			TURN_OFF_CLEAN_STEP_2();
			TURN_OFF_CLEAN_STEP_3();
		break;
				
		case MOTOR_STOP:
			TURN_OFF_CLEAN_STEP_0();
			TURN_OFF_CLEAN_STEP_1();
			TURN_OFF_CLEAN_STEP_2();
			TURN_OFF_CLEAN_STEP_3();
		break;
	}
}

static void Bidet_Motor_Port_Control( void )
{
	switch( StepMotor[TYPE_MOTOR_BIDET].Step )
	{
		case 0:
			TURN_ON_BIDET_STEP_0();
			TURN_OFF_BIDET_STEP_1();
			TURN_OFF_BIDET_STEP_2();
			TURN_ON_BIDET_STEP_3();
		break;
					
		case 1:
            TURN_ON_BIDET_STEP_0();
			TURN_ON_BIDET_STEP_1();
			TURN_OFF_BIDET_STEP_2();
			TURN_OFF_BIDET_STEP_3();
		break;
		
		case 2:
			TURN_OFF_BIDET_STEP_0();
			TURN_ON_BIDET_STEP_1();
			TURN_ON_BIDET_STEP_2();
			TURN_OFF_BIDET_STEP_3();
		break;
			
		case 3:
            TURN_OFF_BIDET_STEP_0();
			TURN_OFF_BIDET_STEP_1();
			TURN_ON_BIDET_STEP_2();
			TURN_ON_BIDET_STEP_3();
		break;
				
		case MOTOR_STOP:
			TURN_OFF_BIDET_STEP_0();
			TURN_OFF_BIDET_STEP_1();
			TURN_OFF_BIDET_STEP_2();
			TURN_OFF_BIDET_STEP_3();
		break;
	}
}

static void Dry_Motor_Port_Control( void )
{
	switch( StepMotor[TYPE_MOTOR_DRY].Step )
	{
		case 0:
			TURN_ON_DRY_STEP_0();
			TURN_OFF_DRY_STEP_1();
			TURN_OFF_DRY_STEP_2();
			TURN_ON_DRY_STEP_3();
		break;
					
		case 1:
			TURN_ON_DRY_STEP_0();
			TURN_ON_DRY_STEP_1();
			TURN_OFF_DRY_STEP_2();
			TURN_OFF_DRY_STEP_3();
		break;
		
		case 2:
			TURN_OFF_DRY_STEP_0();
			TURN_ON_DRY_STEP_1();
			TURN_ON_DRY_STEP_2();
			TURN_OFF_DRY_STEP_3();
		break;
			
		case 3:
			TURN_OFF_DRY_STEP_0();
			TURN_OFF_DRY_STEP_1();
			TURN_ON_DRY_STEP_2();
			TURN_ON_DRY_STEP_3();
		break;
				
		case MOTOR_STOP:
			TURN_OFF_DRY_STEP_0();
			TURN_OFF_DRY_STEP_1();
			TURN_OFF_DRY_STEP_2();
			TURN_OFF_DRY_STEP_3();
		break;
	}
}




