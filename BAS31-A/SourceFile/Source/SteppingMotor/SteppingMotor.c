#include "hw.h"

#include "Mode.h"

#include "SteppingMotor.h"



#define MOTOR_DIRECT_FOWARD       1
#define MOTOR_DIRECT_BACK         0



#define MOTOR_ORIGIN_DELAY_100MSEC        1000      //0.1msec ���� 100msec






typedef struct _steppingmotor_
{
 
    U8 State;           /*�ܼ� level*/
    
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



/* �ش� ��Ʈ�� 0���� */
void ClearMotorEndFlag(U8 mu8Event )
{
    gu8End &= ~mu8Event;
}
/* �ش� ��Ʈ�� 1�� */
void SetMotorEndFlag(U8 mu8Event )
{
    gu8End |= mu8Event;
}
/*�ش� ��Ʈ�� SET �Ǿ� ������ TRUE */
U8 IsSetMotorEndFlag( U8 mu8Event )
{
    if( (gu8End & mu8Event) ==  mu8Event )
    {
        return TRUE;
    }

    return FALSE;
}



/* �ش� ��Ʈ�� 0���� */
void ClearMotorUsedFlag(U8 mu8Event )
{
    gu8Used &= ~mu8Event;
}
/* �ش� ��Ʈ�� 1�� */
void SetMotorUsedFlag(U8 mu8Event )
{
    gu8Used |= mu8Event;
}
/*�ش� ��Ʈ�� SET �Ǿ� ������ TRUE */
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
    FOWARD_TO_BACK_ADD_PULSE,       /* ������->������: �� �� �ڷ� */
    BACK_TO_FOWARD_ADD_PULSE,       /* ������->������: �� �� ������ */
                                    /* ������ Ŭ���� ������ Ŀ�� */

    MOTOR_PULSE_MAX,    
} MotorPulse_T;



#define PRESS_COUNT_MAX     1350
#define PRESS_COUNT_MIN     330

//23: 80�� �� 5mm �ڷ� �з���
//�������� ��� �����ϰ� �ְ�, 3way�� �� �з� ����Ͽ� �ұ��� 
const U16 gu16InitialPulse[TYPE_MAX][MOTOR_PULSE_MAX] = 
{
    /*2018-07-24 �������� ���� �̺��� ����: Init_add_active �߰� */
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
    /*�ڱ⼼ô,  1��,    2��,   3��,    4��,    5�� */
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 1 (Clean)
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 2 (Bidet)
    {0,         610,    680,    740,    800,    860    },   //u8Mode == 3 (Dry) Ȯ���ʿ�
    },

    /*TARGET_STEP_BIDET*/
    {
    /*�ڱ⼼ô,  1��,    2��,   3��,    4��,    5�� */
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 1 (Clean)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 2 (Bidet)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 3 (Dry) Ȯ���ʿ�
    },

    /*TARGET_STEP_PRESS*/
    {
/*�Ϲݼ��� �ڱ⼼ô, ����1(380cc),   ����2(460cc),   ����3(540cc),        ����,   EMPTY */
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 1 (Clean)
/*�������� �ڱ⼼ô, ����1(440cc),   ����2(520cc),   ����3(600cc),        ����,   EMPTY */
            {0,         540,            870,            1170,             1800,    1800     },   //u8Mode == 2 (Bidet)
            {0,         440,            680,            1170,             1800,    1800     },   //u8Mode == 3 (Dry) Ȯ���ʿ�
            },

    /*TARGET_STEP_PRESS_CARE*/
    {
/*�ɾ�� �ڱ⼼ô, ����1(300cc),   ����2(380cc),   ����3(460cc),      ����,   EMPTY */
            {0,         285,                470,        740,            1800,     1800     },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
            {0,         285,                450,        720,            1800,     1800     },   //u8Mode == 1 (Clean)
/*�����ɾ�� �ڱ⼼ô, ����1(380cc),   ����2(460cc),   ����3(540cc),        ����,   EMPTY */
            {0,         415,                530,        800,            1800,     1800     },   //u8Mode == 2 (Bidet)
            {0,         285,                470,        740,            1800,     1800     },   //u8Mode == 3 (Dry) Ȯ���ʿ�
            },

    /*TARGET_STEP_WIDE*/
    {
    /*Origin, CleanWide1, CleanWide1.5 CleanWide2, BidetWide2, BidetWide1,   EMPTY */
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 1 (Clean)
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 2 (Bidet)
    {0,         130,        400,        500,        1420,       1730    },   //u8Mode == 3 (Dry) Ȯ���ʿ�
    },

    /*TARGET_STEP_DRY*/
    {
    /*�ڱ⼼ô,  1��,    2��,   3��,    4��,    5�� */
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 0 (���), �ڱ⼼ô, ����2��, ���⼼�� ���
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 1 (Clean)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 2 (Bidet)
    {0,         720,    780,    840,    900,    970    },   //u8Mode == 3 (Dry) Ȯ���ʿ�
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
    if ( gu8InitReady >= 10 )       //�ʱ�ð� ���� ��
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
    //���� ���¿� ������� ���� �Է½� 60pulse �߰� �̵���
    if( mu83wayState == 0 )		 //�������� �̵��� ���	
    {
        StepMotor[mu8Type].OriginCheck = TRUE;
        ClearMotorUsedFlag(mu8TypeBit);
    }
#endif
    
    if( StepMotor[mu8Type].State != mu83wayState )     //3way ��� ������ ����� �߻��ϸ�
    {
        if( mu83wayState == 0 )		 //�������� �̵��� ���	
        {
            StepMotor[mu8Type].OriginCheck = TRUE;
            ClearMotorUsedFlag(mu8TypeBit);
        }

        SetMotorUsedFlag(mu8TypeBit);
        
        /*�������濡 ���� ���� ���װ� ����*/
        //if ( mu8Type == TYPE_MOTOR_PRESS )
        //{
            //Watt_Compensation_PressLevel( mu83wayState );
        //}
        
        
        
        //������
        if ( StepMotor[mu8Type].Direct == MOTOR_DIRECT_FOWARD )    //������ ���������� �̵��ߴٸ�
        {
            if(StepMotor[mu8Type].State > mu83wayState)    //�����ܼ� > �Է� �ܼ� -> ������
            {
                StepMotor[mu8Type].ForwardToBack = 1;        //���� �������̾��ٰ� ���������� ����� ���
            }
        }
        
        if ( StepMotor[mu8Type].Direct == MOTOR_DIRECT_BACK )     //������ ���������� �̵��ߴٸ� 
        {
            if(StepMotor[mu8Type].State < mu83wayState)      //�����ܼ� < �Է� �ܼ� -> ������
            {
                StepMotor[mu8Type].BackToForward = 1;        //���� �������̾��ٰ� ���������� ����� ���
            }
        }
        
        if ( StepMotor[mu8Type].State < mu83wayState )    //���� �ܼ� < �Է� �ܼ� -> ������
        {
            StepMotor[mu8Type].Direct = MOTOR_DIRECT_FOWARD;                //������
        }
        else                                  // �����ܼ� >= �Է´ܼ� ->������
        {
            StepMotor[mu8Type].Direct = MOTOR_DIRECT_BACK;                //������
        }
        
        StepMotor[mu8Type].State = mu83wayState;												//���� �ܼ��� �Է� �ܼ� ����
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
        		mu16OriginDelay = StepMotor[mu8Type].MoveCount%200;	//200pulse���� delay
        		
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
    		StepMotor[mu8Type].Step	=	MOTOR_STOP;	//����
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


    
	// �Ϲ����� ���¿���
	if ( IsSetMotorFirstFlag(mu8TypeBit) == FALSE )
	{
	    /*�������� �̵��ϴ� ���*/
		if ( StepMotor[mu8Type].State == 0 )
		{
			if ( StepMotor[mu8Type].MoveCount != 0)
			{
				StepMotor[mu8Type].MoveCount--;
				Motor_CW_Step(mu8Type);
			}
			/*���� �̵� �Ϸ�*/
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
					StepMotor[mu8Type].Step	=	MOTOR_STOP;      //�߰� �̵� �Ϸ� �� ���� ����
					SetMotorEndFlag(mu8TypeBit);
				}
			}
		}
        else
        {
            /* Level �Է� ��*/
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
                else                                                //Target�� �����ߴٸ�
                {
                    if(StepMotor[mu8Type].ForwardToBack == TRUE)  //���� �������̾��ٰ� ���������� ����� ���
                    {
                        StepMotor[mu8Type].ForwardToBack = FALSE;
                        StepMotor[mu8Type].MoveCount += gu16InitialPulse[mu8Type][FOWARD_TO_BACK_ADD_PULSE];
                    }
                    else if(StepMotor[mu8Type].BackToForward == TRUE)    //���� �������̾��ٰ� ���������� ����� ���
                    {
                        StepMotor[mu8Type].BackToForward = FALSE;
                        StepMotor[mu8Type].MoveCount -= gu16InitialPulse[mu8Type][BACK_TO_FOWARD_ADD_PULSE];
                    }
                    else
                    {
                        StepMotor[mu8Type].Step =   MOTOR_STOP; //����
                        SetMotorEndFlag(mu8TypeBit);
            
                        /*��ǥ ���� �� ���� ����*/
                        //if ( mu8Type == TYPE_MOTOR_PRESS )
                        //{
                            //Resistor_Compensation_PressLevel();
                        //}
                    }
                }
            }   
            else
            {
                StepMotor[mu8Type].Step =   MOTOR_STOP; //����
                SetMotorEndFlag(mu8TypeBit);
            }
        }
	}
}

/* �����ϴ� ���� */
static void Motor_CW_Step( U8 mu8Type )
{
    /*���������� ���� step���� ����*/
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

/* �����ϴ� ���� */
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




