#pragma interrupt INTTM04 r_tau0_channel4_interrupt


#include "hw.h"
#include "buzzer.h"
#include "eeprom.h"
#include "Setting.h"


/* TIMER */
#define	P_BUZZER_ON		         P_BUZZER_ONOFF
#define	BUZ_SET_NOTE( x )	      do{ TDR04 = (x); }while(0)
#define  BUZZER_TIMER_START()    do{ R_TAU0_Channel4_Start(); }while(0)
#define  BUZZER_TIMER_STOP()     do{ R_TAU0_Channel4_Stop(); TO0 |= _0010_TAU_CH4_OUTPUT_VALUE_1; }while(0)

static void Buz_Start(void)
{
    BUZZER_TIMER_START();
}

static void Buz_Stop(void)
{
    BUZZER_TIMER_STOP();
}
__interrupt static void r_tau0_channel4_interrupt(void)
{
}

/* MAIN 주파수 설정 */
#define	OSC_8MHZ	0
#define	OSC_16MHZ	0
#define	OSC_32MHZ	1

#if (OSC_8MHZ == 1)

/* 4옥타브 */
#define Do4       (15288U)
#define Dos4      (14430U)
#define Re4       (13620U)
#define Mi4       (12134U)
#define Pa4       (11453U)
#define Pas4      (10810U)
#define Sol4      (10203U)
#define Sols4     (9630U)
#define La4       (9090U)
#define Si4       (8098U)

#elif (OSC_16MHZ == 1)

/* 4옥타브 */
#define Do4       (30577U)
#define Dos4      (28861U)
#define Re4       (27241U)
#define Mi4       (24269U)
#define Pa4       (22907U)
#define Pas4      (21621U)
#define Sol4      (20407U)
#define Sols4     (19262U)
#define La4       (18181U)
#define Si4       (16197U)

#else // OSC_32MHZ
/* 4옥타브 */
#define Do4       (61155U)
#define Dos4      (57723U)
#define Re4       (54483U)
#define Mi4       (48539U)
#define Pa4       (45814U)
#define Pas4      (43243U)
#define Sol4      (40816U)
#define Sols4     (38525U)
#define La4       (36363U)
#define Si4       (32395U)
#endif   

/* 5옥타브 */
#define Do5       (Do4 / 2 )
#define Dos5      (Dos4 / 2 ) 
#define Re5       (Re4 / 2)    
#define Mi5       (Mi4 / 2)  
#define Pa5       (Pa4 / 2)  
#define Pas5      (Pas4 / 2)  
#define Sol5      (Sol4 / 2) 
#define Sols5     (Sols4 / 2) 
#define La5       (La4 / 2)
#define Si5       (Si4 / 2)      

/* 6 옥타브 */
#define Do6       (Do4 / 4 )
#define Dos6      (Dos4 / 4 ) 
#define Re6       (Re4 / 4)    
#define Mi6       (Mi4 / 4)  
#define Pa6       (Pa4 / 4)  
#define Pas6      (Pas4 / 4)  
#define Sol6      (Sol4 / 4) 
#define Sols6     (Sols4 / 4) 
#define La6       (La4 / 4)
#define Si6       (Si4 / 4)      

/* 7 옥타브 */
#define Do7       (Do4 / 8 )
#define Dos7      (Dos4 / 8 ) 
#define Re7       (Re4 / 8)    
#define Mi7       (Mi4 / 8)  
#define Pa7       (Pa4 / 8)  
#define Pas7      (Pas4 / 8)  
#define Sol7      (Sol4 / 8) 
#define Sols7     (Sols4 / 8) 
#define La7       (La4 / 8)
#define Si7       (Si4 / 8)      


typedef struct _buuzer_
{
    U8 First;
    
    U8 OnOff;       /* Setting */
    
    U8 Mode;        /*buzzer type*/
    U8 Order;       /*buzzer step*/
    U16 Time;
} Buzzer_T;

Buzzer_T    buzzer;

typedef struct _buzzerlist_
{
    U8 Id;
    
    U16 Freq_1;
    U16 OnTime_1;
    U16 OffTime_1;
    
    U16 Freq_2;
    U16 OnTime_2;
    U16 OffTime_2;
    
    U16 Freq_3;
    U16 OnTime_3;
    U16 OffTime_3;
    
    U16 Freq_4;
    U16 OnTime_4;
    U16 OffTime_4;
    
    U16 Freq_5;
    U16 OnTime_5;
    U16 OffTime_5;

    U16 Freq_6;
    U16 OnTime_6;
    U16 OffTime_6;
     
} BuzzerList_T;

BuzzerList_T BuzzerList[]=
{
    /*id*/                      /*TDR*/ /*Time*/
    { BUZZER_NONE,                 0,      0,  0,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_POWER_ON,             Si5,    4,  19,  Re6,   4,     19,    Sol6,  4,     19,    Pas6,  5,     65,    Re7,   10,    112,   0,     0,     0    },
    { BUZZER_POWER_OFF,            La6,    4,  19,  Sol6,  4,     19,    Mi6,   4,     19,    Pas6,  5,     65,    Re6,   10,    110,   0,     0,     0    },
    { BUZZER_SELECT,               La6,    8,  28,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_SET_ON,               Sol6,   5,  19,  Re7,   10,    38,    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_SET_OFF,              Re7,    5,  19,  Sol6,  10,    38,    Re6,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_WARNING,              Re6,    2,  10,  Re7,   8,     28,    Re6,   2,     10,    Re7,   8,     28,    0,     0,     0,     0,     0,     0    },
    { BUZZER_NOZZLE_CLEAN,         Si5,    6,  18,  Pas6,  6,     18,    Mi6,   6,     42,    0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_1,            La6,    7,  29,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_2,            La6,    7,  29,  La6,   7,     29,    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_3,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    0,     0,     0,     0,     0,     0,     0,     0,     0    },    
    { BUZZER_MEMENTO_4,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    0,     0,     0,     0,     0,     0    },	    
    { BUZZER_MEMENTO_5,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    0,     0,     0    }

#if 0
    { BUZZER_NONE,                 0,      0,  0,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_DRY,                  Pas6,   6,  26,  La5,   6,     10,    Dos6,  6,     30,    Re6,   6,     34,    0,     0,     0,     0,     0,     0    },
    { BUZZER_BIDET,                Re6,    6,  26,  La6,   6,     10,    Mi6,   6,     30,    Pas6,  6,     34,    0,     0,     0,     0,     0,     0    },
    { BUZZER_CLEAN,                Re6,    6,  10,  Mi6,   6,     10,    Pas6,  6,     26,    Re6,   6,     26,    La5,   6,     26,    La6,   6,     26   },
    { BUZZER_SELECT,               La6,    4,  32,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_POWER_ON,             Si5,    4,  19,  Re6,   4,     19,    Sol6,  4,     19,    Pas6,  5,     65,    Re7,   3,     112,   0,     0,     0    },
    { BUZZER_POWER_OFF,            La6,    4,  19,  Sol6,  4,     19,    Mi6,   4,     19,    Pas6,  5,     65,    Re6,   3,     110,   0,     0,     0    },
    { BUZZER_AIRPLUS_ON,           Re6,    4,  28,  Mi6,   4,     12,    La6,   4,     32,    Pas6,  4,     36,    0,     0,     0,     0,     0,     0    },
    { BUZZER_SET_OFF,              Pas6,   4,  20,  La6,   4,     20,    Re6,   4,     44,    0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_SET_ON,               Pas6,   4,  20,  Re7,   4,     20,    Sol6,  4,     20,    La6,   4,     20,    Re7,   4,     44,    0,     0,     0    },
    { BUZZER_MANUAL_STER_START,    Mi6,    4,  20,  Dos6,  4,     20,    La6,   4,     48,    0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MANUAL_STER_END,      La6,    4,  32,  Dos6,  4,     32,    Dos6,  4,     20,    Pas6,  4,     44,    Mi6,   4,     60,    0,     0,     0    },
    { BUZZER_GREENPLUG_ON,         Mi6,    4,  44,  Dos6,  4,     20,    La6,   4,     44,    Sols6, 4,     20,    Mi6,   4,     20,    La6,   4,     44   },
    { BUZZER_GREENPLUG_OFF,        Dos6,   4,  44,  Mi6,   4,     20,    La6,   4,     44,    Sols6, 4,     20,    Mi6,   4,     20,    Dos6,  4,     44   },
    { BUZZER_SPECIAL_MODE,         Pas6,   4,  20,  Re7,   4,     44,    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_WARNING,              Re6,    4,  8,   Re7,   4,     32,    Re6,   4,     8,     Re7,   4,     32,    0,     0,     0,     0,     0,     0    },
    { BUZZER_NOZZLE_CLEAN,         Si5,    6,  18,  Pas6,  6,     18,    Mi6,   6,     42,    0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_AIRPLUS_OFF,          Re7,    6,  28,  Sol6,  6,     28,    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_1,            La6,    7,  29,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_2,            La6,    7,  29,  La6,   7,     29,    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0    },
    { BUZZER_MEMENTO_3,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    0,     0,     0,     0,     0,     0,     0,     0,     0    },    
    { BUZZER_MEMENTO_4,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    0,     0,     0,     0,     0,     0    },	    
    { BUZZER_MEMENTO_5,            La6,    7,  29,  La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    La6,   7,     29,    0,     0,     0    }
#endif
};
#define SZ_BUZZER_LIST ( sizeof(BuzzerList) / sizeof(BuzzerList_T) )

static void BuzzerStop ( void );
static void BuzzerHandler( BuzzerList_T *pList, U8 mu8Size );

void InitBuzzer( void )
{
    buzzer.First = FALSE;
    buzzer.OnOff = TRUE;
    buzzer.Mode = 0;
    buzzer.Order = 0;
    buzzer.Time = 0;

	Buz_Stop();     
	BUZ_SET_NOTE( 0 );
}

U8	GetBuzOnOff(void)
{
	return buzzer.OnOff;
}

void SetBuzOnOff(U8 mu8Mute )
{
	buzzer.OnOff = mu8Mute;
	SaveEepromId( EEP_ID_SOUND );
}

void ToggleBuzOnOff ( void )
{
    if ( buzzer.OnOff == TRUE )
    {
        buzzer.OnOff = FALSE;
    }
    else
    {
        buzzer.OnOff = TRUE;
    }
    SetSettingTime(SETTING_TIME);
    SaveEepromId( EEP_ID_SOUND );
}




/// @brief    Buzzer 종류 설정 함수
/// @param    mu8Step : Buzzer 종류
/// @return   void
void BUZZER_STEP(U8 mu8Step)
{
	buzzer.Time = 0;
	buzzer.Order = 1;
	buzzer.Mode = mu8Step;
}


/// @brief    Buzzer Time Count 제어 함수(10ms)
/// @param    void
/// @return   void
void Evt_10msec_BuzTimeCounter(void)
{
	if (buzzer.Time != 0)
	{ 
		buzzer.Time--;
	}
}

/// @brief    Buzzer 제어 함수
/// @param    void
/// @return   void
void Evt_BuzControl(void)
{
    if ( buzzer.OnOff == TRUE )
    { 
        BuzzerHandler( BuzzerList, SZ_BUZZER_LIST );
    }
    else    // 음소거모드 동작
    {
        /* 2015-01-22. 인천품질. 무음모드 진입 시 무음설정음도 삭제 */
        /* 2016-03-30. Tact SW 모델 무음모드-어린이음 삭제*/
        
        /* 무음모드에서 초기 전원 인가시 전원ON 음성 출력*/
        if  ( buzzer.First == FALSE  )
           // && ( buzzer.Mode == BUZZER_POWER_ON ) )
        {
            BuzzerHandler( BuzzerList, SZ_BUZZER_LIST );
        }
        else
        {
            BuzzerStop();
        }
    }
}


static void BuzzerStop ( void )
{
    if ( buzzer.Mode == BUZZER_POWER_ON )
    {
        buzzer.First = TRUE;
    }
    Buz_Stop();
    BUZ_SET_NOTE ( 0 );
    P_BUZZER_ON = OFF;
    
    buzzer.Time  = 0;
    buzzer.Order = 0;
    buzzer.Mode = 0;
    

}

static void BuzzerHandler( BuzzerList_T *pList, U8 mu8Size )
{
    U8 mu8number;
    
    for ( mu8number = 0; mu8number < mu8Size; mu8number++ )
    {
        if ( buzzer.Mode == ( pList+mu8number )->Id )
        {
            if ( ( buzzer.Order == 0) && ( buzzer.Time == 0))
            {
                BuzzerStop();
            }
            
            if ( ( buzzer.Order == 1 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_1 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_1 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_1;
                    buzzer.Order++;
                }
            }
            
            if ( ( buzzer.Order == 2 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_1;
                buzzer.Order++;
            }
            
            if ( ( buzzer.Order == 3 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_2 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_2 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_2;
                    buzzer.Order++;
                }
            }

            if ( ( buzzer.Order == 4 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_2;
                buzzer.Order++;
            }
            
            if ( ( buzzer.Order == 5 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_3 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_3 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_3;
                    buzzer.Order++;
                }
            }

            if ( ( buzzer.Order == 6 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_3;
                buzzer.Order++;
            }
            
            if ( ( buzzer.Order == 7 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_4 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_4 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_4;
                    buzzer.Order++;
                }
            }

            if ( ( buzzer.Order == 8 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_4;
                buzzer.Order++;
            }
            
            if ( ( buzzer.Order == 9 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_5 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_5 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_5;
                    buzzer.Order++;
                }
            }

            if ( ( buzzer.Order == 10 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_5;
                buzzer.Order++;
            }
            if ( ( buzzer.Order == 11 ) && ( buzzer.Time == 0 ) )
            {
                if ( ( pList+mu8number )->Freq_6 == 0 )
                {
                    BuzzerStop();
                }
                else
                {
                    Buz_Start();  //buzzer start
                    BUZ_SET_NOTE ( ( pList+mu8number )->Freq_6 );
                    P_BUZZER_ON = ON;
                    buzzer.Time = ( pList+mu8number )->OnTime_6;
                    buzzer.Order++;
                }
            }

            if ( ( buzzer.Order == 12 ) && ( buzzer.Time == 0 ) )
            {
                P_BUZZER_ON = OFF;
                buzzer.Time = ( pList+mu8number )->OffTime_6;
                buzzer.Order++;
            }
            
            if ( ( buzzer.Order == 13) && ( buzzer.Time == 0))
            {
                BuzzerStop();
            }
            break; 
        }
    }
}

