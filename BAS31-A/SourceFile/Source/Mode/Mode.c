#include "hw.h"

#include "Mode.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "Setting.h"
#include "Sterilize.h"
#include "Adc.h"
#include "Hotwater.h"

#include "Valve.h"
#include "Dry.h"
#include "Air_Plus.h"
#include "Waterheater.h"
#include "SteppingMotor.h"
#include "Sterilize.h"
#include "WaterLevel.h"
#include "WaterTank.h"
#include "Powersave.h"

#include "FactoryMode.h"
#include "Aging.h"


#define PRE_HEATING_PASS_DELAY  50      /*10msec * 10 * 5 = 0.5sec */
#define PRE_HEATING_DELAY       200      /*10msec * 10 * 20 = 2sec */


typedef struct _do_type_val_
{
    /* Nozzle */
    U8 TypeMotor;    // TYPE_MOTOR_CLEAN or TYPE_MOTOR_BIDET
    U8 TypeMotorBit;
    U8 Nozzle_Level;

    /* Wide */
    U8 Drain_Wide;
    U8 Start_Wide;
    U8 Pre_Wide;
    U8 Main_Wide;
    U8 Rinse_Wide;

    /* Time */
    U16 Start_Time;
    U16 Pre_Time;
    U16 Main_Time;
    U16 Rinse_Time;

    /* Press */
    U8 Start_Press;
    U8 Pre_Press;
    U8 Main_Press;
    U8 Rinse_Press;

    /* Function */
    U8 Air_Enable_PreWash_Clean;
    U8 Air_Enable_PreWash_Bidet;
    U8 Air_Enable_MainWash;
    U8 BodySter_Enable;
    U8 Move_Enable;
    
} DoTypeValue_T;

DoTypeValue_T DoTypeValueTable[] =              
{
/* StateId_T */
                                                            
    /* TypeMotor,        TypeMotorBit,     Nozzle_Level,                        Wide(Drain/Start/Pre/Main/Rinse),                         Time(Start/Pre/Main/Rinse),  Press(Start/Pre/Main/Rinse),  Air_Enable_PreWash_Clean,  Air_Enable_PreWash_Bidet, Air_Enable_MainWash,  BodySter_Enable,  Move_Enable  */
//TABLE_CLEAN_NORMAL
    { TYPE_MOTOR_CLEAN,  TYPE_BIT_MOTOR_CLEAN,    3,        WIDE_CLEAN_2,   WIDE_CLEAN_1_5,  WIDE_CLEAN_2,    WIDE_CLEAN_1,    WIDE_CLEAN_1,        300,    1200,    4000,    500,        1,   3,   3,   3,           FALSE,                    FALSE,                      TRUE,               TRUE,           FALSE   }, 

//TABLE_CLEAN_ACTIVE
    { TYPE_MOTOR_CLEAN,  TYPE_BIT_MOTOR_CLEAN,    3,        WIDE_CLEAN_2,   WIDE_CLEAN_1_5,  WIDE_CLEAN_2,    WIDE_CLEAN_1,    WIDE_CLEAN_1,        300,    1200,    4000,    500,        1,   3,   3,   3,           FALSE,                    FALSE,                      TRUE,               TRUE,           TRUE    },

//TABLE_CLEAN_CARE
    { TYPE_MOTOR_CLEAN,  TYPE_BIT_MOTOR_CLEAN,    3,        WIDE_CLEAN_2,   WIDE_CLEAN_1_5,  WIDE_CLEAN_2,    WIDE_CLEAN_1,    WIDE_CLEAN_1,        300,    2000,    2700,    1000,       1,   1,   1,   2,           TRUE,                     FALSE,                      TRUE,               TRUE,           FALSE   }, 

//TABLE_BIDET_NORMAL
    { TYPE_MOTOR_BIDET,  TYPE_BIT_MOTOR_BIDET,    5,        WIDE_BIDET_2,   WIDE_BIDET_1,    WIDE_BIDET_1,    WIDE_BIDET_1,    WIDE_BIDET_1,        300,    0,       5400,    300,        1,   3,   3,   1,           FALSE,                    FALSE,                      FALSE,              FALSE,          FALSE   },

//TABLE_BIDET_ACTIVE
    { TYPE_MOTOR_BIDET,  TYPE_BIT_MOTOR_BIDET,    5,        WIDE_BIDET_2,   WIDE_BIDET_1,    WIDE_BIDET_1,    WIDE_BIDET_1,    WIDE_BIDET_1,        300,    0,       5400,    300,        1,   3,   3,   1,           FALSE,                    FALSE,                      FALSE,              FALSE,          TRUE    },

//TABLE_BIDET_CARE
    { TYPE_MOTOR_BIDET,  TYPE_BIT_MOTOR_BIDET,    5,        WIDE_BIDET_2,   WIDE_BIDET_2,    WIDE_BIDET_2,    WIDE_BIDET_2,    WIDE_BIDET_2,        300,    700,     4000,    1000,       1,   3,   2,   2,           FALSE,                    FALSE,                      TRUE,               FALSE,          FALSE   }


};







typedef U8(*Do_T)( DoTypeValue_T *pVal );
typedef void(*Initial_T)(void);

typedef struct _event_
{
    U8                      StepMode;
    Do_T                    DoAction;           //Do Function
    Initial_T               InitialAction;      //Initial Function
} EventList_T;

static U8 DoBefore( DoTypeValue_T *pVal );
static U8 DoMain( DoTypeValue_T *pVal );
static U8 DoCleanAfter( DoTypeValue_T *pVal );
static U8 DoBidetAfter( DoTypeValue_T *pVal );
static U8 DoAfterWithStep( After_Step_T *pStep, DoTypeValue_T *pVal );

EventList_T    CleanEventList[] =
{
/*      StepMode,               Function,       Initial,            */
    { CLEAN_MODE_BEFORE,      DoBefore,       InitBefore      },
    { CLEAN_MODE_MAIN,        DoMain,         InitMain        },
    { CLEAN_MODE_AFTER,       DoCleanAfter,   InitCleanAfter  }
};
#define SZ_CLEAN_EVENT_LIST ( sizeof(CleanEventList) / sizeof(EventList_T) )

EventList_T    BidetEventList[] =
{
/*      StepMode,               Function,       Initial,            */
    { BIDET_MODE_BEFORE,      DoBefore,       InitBefore      },
    { BIDET_MODE_MAIN,        DoMain,         InitMain        },
    { BIDET_MODE_AFTER,       DoBidetAfter,   InitBidetAfter  }
};
#define SZ_BIDET_EVENT_LIST ( sizeof(BidetEventList) / sizeof(EventList_T) )



/*����ü �����*/
DoBefore_Step_T         Before;
DoMain_Step_T           Main;
DoAfter_Step_T          After;

/*���μ��� �ʱ�ȭ*/
static void InitBefore(void)
{
    Before.Step = 0;
}
static void InitMain(void)
{
    Main.Step = 0;
}
static void InitCleanAfter(void)
{
    After.CleanStep = 0;
}
static void InitBidetAfter(void)
{
    After.BidetStep = 0;
}




typedef struct _mode_
{
    U8 Sel;             /*���/����/��/����*/
    U8 Flag;            /*��� ���� ���� flag*/
    U8 State;          /* Normal / Care / Active ���� */
    U8 StateDefault;    /* Normal / Care / Active Default ���� */
    
    U8 CleanMode;        /* ����ô/���μ���/�ļ�ô Mode bit */
    U8 BidetMode;        /* ����ô/���μ���/�ļ�ô Mode bit */

    U16 Time;
    
    U16 ForeceTimer;
    U16 Delay;
    
    U8 Change;
    U8 SterPass;
} Mode_T;

Mode_T Mode;

void InitMode(void)
{
    Mode.Sel = TYPE_WAIT;           /* flag ���� ���� Mode ����*/
    Mode.Flag = MODE_FLAG_NONE;     /* Key �Է¿� ���� flag */
    Mode.State = STATE_BASIC;
    Mode.StateDefault = STATE_BASIC;
    
    Mode.CleanMode = 0;         /* ����ô/���μ���/�ļ�ô Mode bit */
    Mode.BidetMode = 0;         /* ����ô/���μ���/�ļ�ô Mode bit */
    
    Mode.Time = 0;
    
    Mode.ForeceTimer = 0;       /* ���� ���� ���� �ð� */
    Mode.Delay = 0;             /* pre heating delay */
    
    Mode.Change = FALSE;
    Mode.SterPass = FALSE;
}

void SetModeSel( U8 mu8val)
{
    Mode.Sel = mu8val;
}

U8 GetModeSel(void)
{
    return Mode.Sel;
}

void SetModeState( U8 mu8val)
{
    Mode.State = mu8val;
}

U8 GetModeState ( void )
{
    return Mode.State;
}

void SetModeStateDefault( U8 mu8val)
{
    Mode.StateDefault = mu8val;
}

U8 GetModeStateDefault ( void )
{
    return Mode.StateDefault;
}

void SetModeFlag(U8 mu8Mode)
{
    Mode.Flag |= mu8Mode;
}

U8 GetModeFlag(void)
{
    return Mode.Flag;
}

void ClearModeFlag(U8 mu8Mode)
{
    Mode.Flag &= ~mu8Mode;
}

U8 IsSetModeFlag(U8 mu8Mode)
{
    if ( ( Mode.Flag & mu8Mode ) == mu8Mode )
    {
        return TRUE;
    }
    
    return FALSE;
}


void SetModeTime ( U16 mu16val)
{
    Mode.Time = mu16val;
}

U16 GetModeTime()
{
    return Mode.Time;
}



void SetCleanMode(U8 mu8Mode)
{
    Mode.CleanMode |= mu8Mode;
}

U8 GetCleanMode(void)
{
    return Mode.CleanMode;
}

void ClearCleanMode(U8 mu8Mode)
{
    Mode.CleanMode &= ~mu8Mode;
}

U8 IsSetCleanMode(U8 mu8Mode)
{
    if ( ( Mode.CleanMode & mu8Mode ) == mu8Mode )
    {
        return TRUE;
    }
    
    return FALSE;
}

void SetBidetMode(U8 mu8Mode)
{
    Mode.BidetMode |= mu8Mode;
}

U8 GetBidetMode(void)
{
    return Mode.BidetMode;
}

void ClearBidetMode(U8 mu8Mode)
{
    Mode.BidetMode &= ~mu8Mode;
}

U8 IsSetBidetMode(U8 mu8Mode)
{
    if ( ( Mode.BidetMode & mu8Mode ) == mu8Mode )
    {
        return TRUE;
    }
    
    return FALSE;
}



void SetBeforeStep ( Before_Step_T val )
{
    Before.Step = val;
}


void SetMainStep ( Main_Step_T val )
{
    Main.Step = val;
}



Before_Step_T GetBeforeStep(void)
{
    return Before.Step;
}
Main_Step_T GetMainStep(void)
{
    return Main.Step;
}
After_Step_T GetAfterCleanStep(void)
{
    return After.CleanStep;
}
After_Step_T GetAfterBidetStep(void)
{
    return After.BidetStep;
}


static void CleanEventHandler (EventList_T *pList, U8 mu8Size);
static void BidetEventHandler (EventList_T *pList, U8 mu8Size);
static void ModeChange ( void );
static void CleanStart(void);
static void BidetStart(void);
static void DryStart(void);
static void StopFunction(void);






/*10msec*/
void Evt_10msec_Mode_Step()
{
    CleanEventHandler( CleanEventList, SZ_CLEAN_EVENT_LIST );   /*PROCESS CONTROL*/
    BidetEventHandler( BidetEventList, SZ_BIDET_EVENT_LIST );   /*PROCESS CONTROL*/
    ModeChange();
}

static void CleanEventHandler (EventList_T *pList, U8 mu8Size)
{
    U8 i;
    
    for ( i = 0; i < mu8Size; i++ )
    {
        if (IsSetCleanMode((pList+i)->StepMode) == TRUE )
        {
            if ( Mode.State == STATE_ACTIVE )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_CLEAN_ACTIVE] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearCleanMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            else if ( Mode.State == STATE_CARE )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_CLEAN_CARE] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearCleanMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            else //if ( Mode.State == STATE_NORMAL )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_CLEAN_NORMAL] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearCleanMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            break;
        }
    }
}

static void BidetEventHandler (EventList_T *pList, U8 mu8Size)
{
    U8 i;
    
    for ( i = 0; i < mu8Size; i++ )
    {
        if (IsSetBidetMode((pList+i)->StepMode) == TRUE )
        {
            if ( Mode.State == STATE_ACTIVE )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_BIDET_ACTIVE] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearBidetMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            else if ( Mode.State == STATE_CARE )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_BIDET_CARE] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearBidetMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            else //if ( Mode.State == STATE_NORMAL )
            {
                if ( (pList+i)->DoAction( &DoTypeValueTable[TABLE_BIDET_NORMAL] ) == TRUE )     //Do function
                {
                    if ( (pList+i)->InitialAction != NULL )
                    {
                        (pList+i)->InitialAction();               // Initial Function
                        ClearBidetMode( (pList+i)->StepMode );  // Mode Clear
                    }
                }
            }
            break;
        }
    }
}

static void ModeChange ( void )
{
    if ( IsSetModeFlag( MODE_FLAG_CLEAN_NORMAL ) == TRUE
        || IsSetModeFlag( MODE_FLAG_CLEAN_CARE ) == TRUE
        || IsSetModeFlag( MODE_FLAG_CLEAN_ACTIVE ) == TRUE )
    {
        CleanStart();
    }
    else if ( IsSetModeFlag( MODE_FLAG_BIDET_NORMAL ) == TRUE
        || IsSetModeFlag( MODE_FLAG_BIDET_CARE ) == TRUE
        || IsSetModeFlag( MODE_FLAG_BIDET_ACTIVE ) == TRUE )
    {
        BidetStart();
    }
    else if ( IsSetModeFlag( MODE_FLAG_DRY ) == TRUE )
    {
        DryStart();
    }
    else if ( IsSetModeFlag( MODE_FLAG_STOP ) == TRUE )
    {
        StopFunction();
        StopNozzleClean();
    }
    else
    {
    }
    
    ClearModeFlag ( MODE_FLAG_ALL );
}

static void CleanStart (void)
{
/******************************************************************************/
/*Clean -> Clean*/
/******************************************************************************/
    if ( Mode.Sel == TYPE_CLEAN )
    {
        if( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE )
        {

        }
        else if( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE )
        {
            if ( GetMainStep() < STEP_MAIN_PRE_SETTING )
            {
                /* 2018-04-02 BX �Ѽ����� ��û ���� */
                /* soft start ���¿��� ���� �Է½� ���� */
                //SetMainStep ( STEP_MAIN_START_SETTING );
            }
            else if ( GetMainStep() >= STEP_MAIN_PRE_SETTING 
                && GetMainStep() < STEP_MAIN_MAIN_SETTING )
            {
                SetMainStep ( STEP_MAIN_PRE_SETTING );
            }
            else
            {
                SetMainStep ( STEP_MAIN_MAIN_SETTING );
            }
        }
        else if( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE )
        {
            SetCleanMode( CLEAN_MODE_ALL );
            SetBeforeStep( STEP_BEFORE_SELF_DRAIN_VALVE_OFF );
            InitCleanAfter();
            EW_Stop();
        }
    }
    else
    {
        Mode.SterPass = FALSE;
      
        SetMoveStart ( FALSE );
        SetAirPlusCmd ( FALSE );

        /******************************************************************************/
        /*Bidet -> Clean*/
        /******************************************************************************/
        if ( Mode.Sel == TYPE_BIDET )
        {
            EW_Stop();

            //�ܼ����� or ���� �� ����
            //����ô �� �����ϸ� �ļ�ô ����
            if ( IsSetBidetMode(BIDET_MODE_BEFORE) == TRUE )
            {
                ClearBidetMode( BIDET_MODE_BEFORE_MAIN );
                InitBefore();
                InitMain();
                Mode.Change = TRUE;
            }
            else if ( IsSetBidetMode(BIDET_MODE_MAIN) == TRUE )
            {
                ClearBidetMode( BIDET_MODE_MAIN );
                InitMain();
                Mode.Change = TRUE;
            }
            //�ļ�ô �� ��������
            else if ( IsSetBidetMode(BIDET_MODE_AFTER) == TRUE )
            {
                SetCleanMode( CLEAN_MODE_ALL );
                InitCleanAfter();
                EW_Stop();
                Mode.Change = FALSE;
            }
            else {}

            Mode.SterPass = TRUE;

        }
        /******************************************************************************/
        /*Dry -> Clean*/
        /******************************************************************************/
        else if ( Mode.Sel == TYPE_DRY )
        {
            SetCleanMode( CLEAN_MODE_ALL );
            InitCleanAfter();
            SetDryTime(0);
        }
        /******************************************************************************/
        /*Wiat -> Clean*/
        /******************************************************************************/
        else //if ( Mode.Sel == TYPE_WAIT )
        {
            SetCleanMode( CLEAN_MODE_ALL );
            InitCleanAfter();
        }
    }
    Mode.Sel = TYPE_CLEAN;
}

static void BidetStart (void)
{
    /******************************************************************************/
    /*Bidet -> Bidet*/ //�� ���� �� �� ��ư �Է� or ����ũ�ν�
    /******************************************************************************/
    if ( Mode.Sel == TYPE_BIDET )
    {
        if (IsSetBidetMode( BIDET_MODE_BEFORE ) == TRUE )
        {

        }
        else if (IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE )         //�� �ð� ����
        {
            if ( GetMainStep() < STEP_MAIN_PRE_SETTING )
            {
                /* 2018-04-02 BX �Ѽ����� ��û ���� */
                /* soft start ���¿��� ���� �Է½� ���� */
                //SetMainStep ( STEP_MAIN_START_SETTING );
            }
            else if ( GetMainStep() >= STEP_MAIN_PRE_SETTING 
                && GetMainStep() < STEP_MAIN_MAIN_SETTING )
            {
                SetMainStep ( STEP_MAIN_PRE_SETTING );
            }
            else
            {
                SetMainStep ( STEP_MAIN_MAIN_SETTING );
            }
        }
        else if (IsSetBidetMode( BIDET_MODE_AFTER ) == TRUE )       // �ܼ����� ���� �� 1��
        {
            SetBidetMode ( BIDET_MODE_ALL );
            SetBeforeStep( STEP_BEFORE_SELF_DRAIN_VALVE_OFF );
            InitBidetAfter();
            EW_Stop();
        }
        else
        {
        }
    }
    else
    {
        Mode.SterPass = FALSE;

        SetMoveStart ( FALSE );
        SetAirPlusCmd ( FALSE );

        /******************************************************************************/
        /*Clean -> Bidet*/
        /******************************************************************************/
        if ( Mode.Sel == TYPE_CLEAN )
        {
            EW_Stop();
            
            if ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE )
            {
                ClearCleanMode( CLEAN_MODE_BEFORE_MAIN );
                InitBefore();
                InitMain();
                Mode.Change = TRUE;
            }
            else if ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE )
            {
                ClearCleanMode( CLEAN_MODE_MAIN );
                InitMain();
                Mode.Change = TRUE;
            }
            else if ( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE )
            {
                SetBidetMode ( BIDET_MODE_ALL );
                InitBidetAfter();
                EW_Stop();
                Mode.Change = FALSE;
            }
            else {}
            
            Mode.SterPass = TRUE;
        }
        /******************************************************************************/
        /*Dry -> Bidet*/
        /******************************************************************************/
        else if ( Mode.Sel == TYPE_DRY )
        {
            SetBidetMode ( BIDET_MODE_ALL );
            InitBidetAfter();
            SetDryTime(0);
        }
        /******************************************************************************/
        /*Wait -> Bidet*/
        /******************************************************************************/
        else //if (Mode.Sel == TYPE_WAIT )
        {
            SetBidetMode ( BIDET_MODE_ALL );
            InitBidetAfter();
        }
    }
    Mode.Sel = TYPE_BIDET;
}


static void DryStart(void)
{
    /*Function change*/
    if ( Mode.Sel == TYPE_CLEAN
        || Mode.Sel == TYPE_BIDET )
    {
        StopFunction();
    }

    if( GetDryService() == FALSE )     //�Ϲݸ���
    {
        SetDryTime(DRY_TIME);
    }
    else                  //���� ����
    {
        SetDryTime(DRY_SERVICE_TIME);//4��
    }

    SetDryStart(TRUE);
    Mode.Sel = TYPE_DRY;
}


static void StopFunction ( void )
{
    Mode.SterPass = FALSE;
    Mode.Change = FALSE;

    SetMoveStart ( FALSE );
    SetAirPlusCmd ( FALSE );

    /*Clean Mode*/
	if ( Mode.Sel == TYPE_CLEAN )
	{
        if ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE )
        {
            ClearCleanMode( CLEAN_MODE_BEFORE_MAIN );
            InitBefore();
            InitMain();
            Mode.SterPass = TRUE;
        }
        else if ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE )
        {
            ClearCleanMode( CLEAN_MODE_MAIN );
            InitMain();
            Mode.SterPass = FALSE;
        }
        else if ( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE )
        {
            Mode.SterPass = FALSE;
        }
        else {}

	}	 
    /*Bidet Mode*/
	else if ( Mode.Sel == TYPE_BIDET )
	{
        //�ܼ����� or ���� �� ����
        //����ô �� �����ϸ� �ļ�ô ����
        if ( IsSetBidetMode(BIDET_MODE_BEFORE) == TRUE )
        {
            ClearBidetMode( BIDET_MODE_BEFORE_MAIN );
            InitBefore();
            InitMain();
            Mode.SterPass = TRUE;
        }
        else if ( IsSetBidetMode(BIDET_MODE_MAIN) == TRUE )
        {
            ClearBidetMode( BIDET_MODE_MAIN );
            InitMain();
            Mode.SterPass = FALSE;
        }
        //�ļ�ô �� ��������
        else if ( IsSetBidetMode(BIDET_MODE_AFTER) == TRUE )
        {
            Mode.SterPass = FALSE;
        }
        else {}

	}
    /*Dry Mode*/
	else if ( Mode.Sel == TYPE_DRY )
	{
		SetDryTime(0);
		Mode.Sel = TYPE_WAIT;   //���� ���� �� StopFunction() ���������� u8Mode ����
		
	}
    else //if ( Mode.Sel == TYPE_WAIT )
    {
    }
}


static U8 DoBefore( DoTypeValue_T *pVal )
{
    U8 mu8Done = FALSE;
    
    switch ( Before.Step )
    {
        case STEP_BEFORE_NOZZLE_CHECK :
            
            //2015-01-18  �ļ�ô �� ���� �Է� �� EW ����
            if ( GetSterAfter() == TRUE )
			{
			    EW_Stop();
			}

            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
            {
                Before.Step++;

                if ( IsSetMotorUsedFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE )
                {
                    //2014-12-04 ���缺 Jr. ����¡��忡�� ����->�� ��ȯ�� �� ��鼭 ���°� ����
                    //if ( GetAgingMode() != AGING_MODE_REPEAT )
                    //{
                        Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );    //�����������
                    //}
                    //else
                    //{
                    //    Before.Step = STEP_BEFORE_NOZZLE_CHECK;
                    //}
                    
                }
                if ( IsSetMotorUsedFlag(TYPE_BIT_MOTOR_BIDET) == TRUE )
                {
                    Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );    //�񵥳������
                }

                if ( IsSetMotorUsedFlag(TYPE_BIT_MOTOR_DRY) == TRUE )
                {
                    Motor_State( (U8) TYPE_MOTOR_DRY, 0, (U8) TYPE_BIT_MOTOR_DRY );    //�����������
                }
            }
        break;

  		case STEP_BEFORE_PRE_HEATING:
  		    if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE )
            {
                /*2016-06-21. ���ȯ1�� ������ ���� ���� ���� ���ۿ��� �ļ�ô�ȵǴ� ���� ����*/
                //SetFlowStop( FALSE );
  				
  				if ( GetFactoryTest() == FALSE )
  				{
                    /*10sec�� �µ� ���� ���ҽ� ���� ����*/
                    if ( Mode.ForeceTimer < PRE_HEATING_PASS_DELAY )
                    {
                        Mode.ForeceTimer++;
                    }
                    
                    if ( ( GetOutSensor() > GetHotTarget() )
                        || ( Mode.ForeceTimer >= PRE_HEATING_PASS_DELAY ) )
                    {
                        /* 2018-07-24 �ڱ⼼ô ���� ��ȭ�� ���� ���� 1��->2�� */
                        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_2, (U8) TYPE_BIT_MOTOR_PRESS );          //���� 2��
                        //Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, (U8) TYPE_BIT_MOTOR_WIDE );      /*����/�� Wide ����*/

                        /* 2018-09-21 �����ɾ��..����.. ����.. */
                        Motor_State( TYPE_MOTOR_WIDE, pVal->Drain_Wide, (U8) TYPE_BIT_MOTOR_WIDE );      /*����/�� Wide ����*/
                        Before.Step++;
                    }
  				}
  				else
  				{
  				    Before.Step = STEP_BEFORE_SELF_DRAIN_NOZZLE;
      			}
            }
  		break;

        case STEP_BEFORE_SELF_PRESS_MOVE:
            if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                // Wide �̵� �Ϸ� ��ٸ��� ����ð� NG
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE ) == TRUE )
            {
                /* 2018-09-21 �����ɾ��..����.. ����.. */
                Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, (U8) TYPE_BIT_MOTOR_WIDE );      /*����/�� Wide ����*/

                Mode.Delay = PRE_HEATING_DELAY;    //2��
                Before.Step++;
            }

        break;

/* Valve ON*/
        case STEP_BEFORE_SELF_DRAIN_VALVE_ON:
            if ( Mode.Delay != 0)
            {
                Mode.Delay--;
            }
            else
            {
                Before.Step++;
            }
        break;

/* Valve OFF */
        case STEP_BEFORE_SELF_DRAIN_VALVE_OFF:
            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
            {
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Before.Step++;
            }
        break;

        case STEP_BEFORE_SELF_DRAIN_NOZZLE:
            if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                /* 2018-07-26. ���ȯ ���� ���� ���� pulse ���� ���� ���� ���� ��(MoveCount != 0) ��� ���ý� ���� ���� ���� �ȵǴ� ���� ���� */
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE )
            {
                Motor_State( pVal->TypeMotor, pVal->Nozzle_Level, pVal->TypeMotorBit );       /*����/�� ���� ����*/
                SetNozzleLevel ( pVal->Nozzle_Level );

                /*2018-07-24 ���缺. �ļ�ô 4way ���� ���¿��� �������� �Է½� ���� ���� -> 4way ��ġ ���� �̵��Ͽ� ���ؼ� */
                Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, (U8) TYPE_BIT_MOTOR_WIDE );

                Before.Step++;
            }
        break;

  		case STEP_BEFORE_READY:
  			if( IsSetMotorEndFlag ( TYPE_BIT_MOTOR_CLEAN ) == TRUE 
                && IsSetMotorEndFlag ( TYPE_BIT_MOTOR_BIDET ) == TRUE  
                && IsSetMotorEndFlag ( TYPE_BIT_MOTOR_WIDE ) == TRUE  )
  			{
		        Before.Step++;
  			}
  		break;
  		
  		case STEP_BEFORE_DONE:
            mu8Done = TRUE;
        break;
        
        default:
        break;
    }
    return mu8Done;
}
static U8 DoMain( DoTypeValue_T *pVal )
{
    U8 mu8Done = FALSE;

    U8 mu8PressLevel;
    mu8PressLevel = GetWaterPressLevel();
    
    if ( Mode.Time != 0 )
    {
        Mode.Time--;
    }
    
    switch ( Main.Step )
    {
        //case 7
		//3way ����->  Water_Pressure_Control();
  		  
  		case STEP_MAIN_START_SETTING:

            Motor_State( TYPE_MOTOR_PRESS, pVal->Start_Press, TYPE_BIT_MOTOR_PRESS);
            //SetWaterPressLevel ( pVal->Start_Press );

            Motor_State( TYPE_MOTOR_WIDE, pVal->Start_Wide, TYPE_BIT_MOTOR_WIDE);           

            /*Body Clean*/
            if ( pVal->BodySter_Enable == TRUE 
                && GetSterSettingBody() == TRUE
                && GetFactoryTest() == FALSE
                && GetAgingMode() != AGING_MODE_CONTINUE )
            {
                /* 2sec */
                SterBodyStart( STERILIZE_BODY_START_TIME );
            }
            else
            {
                SetSterBody ( FALSE );
            } 

            Mode.Time =  pVal->Start_Time;

            /* 2018-06-27 ������ �����δ� �����׽�Ʈ��� ���Խ� ���а��� -> Main Wash���� ���� */
            if ( GetFactoryTest() == FALSE )
            {
                Main.Step++;
            }
            else
            {
                Main.Step = STEP_MAIN_MAIN_SETTING;
            }
            
        break;

        case STEP_MAIN_START_ING:
            
  		    SetSterCountEnable ( TRUE );


            Motor_State( TYPE_MOTOR_PRESS, pVal->Start_Press, TYPE_BIT_MOTOR_PRESS);
#if 0
            /*2018-08-02 �������� ���� �� SoftStart -> PreWash ��ȯ�� ���� �µ� �ϰ� ���� -> SoftStart���� PreWash Wide�� �̵� */
            if ( Mode.Time < 100 )
            {
                Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, TYPE_BIT_MOTOR_WIDE);           
            }
            else
            {
                Motor_State( TYPE_MOTOR_WIDE, pVal->Start_Wide, TYPE_BIT_MOTOR_WIDE);
            }
#endif
            Motor_State( TYPE_MOTOR_WIDE, pVal->Start_Wide, TYPE_BIT_MOTOR_WIDE);

  		    if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                 && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  			{
  			    //F_TemperatureCorrection = 0;
  			    //SetFlowPressCompensationCount(0);   //���� ���� Ƚ��

                if ( Mode.Time == 0 )
                {
  			        Main.Step++;
                }
		    }
  			  
  		break;

        case STEP_MAIN_PRE_SETTING:
            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Pre_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Pre_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }
            
            Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, TYPE_BIT_MOTOR_WIDE);            
            
            Mode.Time = pVal->Pre_Time;
            Main.Step++;
        break;

        case STEP_MAIN_PRE_ING:
            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Pre_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Pre_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }
            
            Motor_State( TYPE_MOTOR_WIDE, pVal->Pre_Wide, TYPE_BIT_MOTOR_WIDE);

            /* Air Plus */
            if ( pVal->Air_Enable_PreWash_Clean == TRUE 
                || pVal->Air_Enable_PreWash_Bidet == TRUE )
            {   
                SetAirPlusCmd ( TRUE );   
            }
            else
            {
                SetAirPlusCmd ( FALSE );
            }
            

            /* Move */
            if ( pVal->Move_Enable == TRUE )
            {            
                SetMoveStart ( TRUE );  
            }
            else
            {
                SetMoveStart ( FALSE );
            }
             
            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  			{
                if ( Mode.Time == 0 )
                {
  			        Main.Step++;
                }
            }
        break;

        case STEP_MAIN_MAIN_SETTING:
            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Main_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Main_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }

            Motor_State( TYPE_MOTOR_WIDE, pVal->Main_Wide, TYPE_BIT_MOTOR_WIDE);            

            Mode.Time = pVal->Main_Time;
            Main.Step++;
        break;

        case STEP_MAIN_MAIN_ING:
            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Main_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Main_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }

            Motor_State( TYPE_MOTOR_WIDE, pVal->Main_Wide, TYPE_BIT_MOTOR_WIDE);

            /* Air Plus */
            if ( pVal->Air_Enable_MainWash == TRUE )
            {
		        SetAirPlusCmd ( TRUE );
            }
            else
            {
                SetAirPlusCmd ( FALSE );
            }    

            /* Move */
            if ( pVal->Move_Enable == TRUE )
            {            
                SetMoveStart ( TRUE );  
            }
            else
            {
                SetMoveStart ( FALSE );
            }

#if CONFIG_INFINITY_MODE
            Mode.Time = pVal->Main_Time;
#endif
            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  			{
                if ( Mode.Time == 0 )
                {
  			        Main.Step++;
                }
            }
        break;

        case STEP_MAIN_RINSE_SETTING:
            SetAirPlusCmd ( FALSE );

            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Rinse_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Rinse_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }

            Motor_State( TYPE_MOTOR_WIDE, pVal->Rinse_Wide, TYPE_BIT_MOTOR_WIDE);            

            /*Body Clean*/
            if ( pVal->BodySter_Enable == TRUE 
                && GetSterSettingBody() == TRUE )
            {
                /* 1sec */
                SterBodyStart( STERILIZE_BODY_RINSE_TIME );
            }
            else
            {
                SetSterBody ( FALSE );
            } 

            Mode.Time = pVal->Rinse_Time;
            Main.Step++;
        break;

        case STEP_MAIN_RINSE_ING:
            SetAirPlusCmd ( FALSE );
           
            /* Move */
            if ( pVal->Move_Enable == TRUE )
            {            
                SetMoveStart ( TRUE );  
            }
            else
            {
                SetMoveStart ( FALSE );
            }

            if ( GetWaterPressUserPress() == FALSE )
            {
                Motor_State( TYPE_MOTOR_PRESS, pVal->Rinse_Press, TYPE_BIT_MOTOR_PRESS);
                SetWaterPressLevel ( pVal->Rinse_Press );
            }
            else
            {
                Motor_State( TYPE_MOTOR_PRESS, mu8PressLevel, TYPE_BIT_MOTOR_PRESS);
            }

            Motor_State( TYPE_MOTOR_WIDE, pVal->Rinse_Wide, TYPE_BIT_MOTOR_WIDE);

            if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
  			{
                if ( Mode.Time == 0 )
                {
                    /*2016-05-25. ���ȯ 1��. ���� �������� ���� �Է½� */
      				/*������ ���� �ƴ� ���¿���(100msec����) ����鼭 ���� ���� ����*/
      				Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                    SetWaterPressUserPress ( FALSE );
                    SetMoveStart ( FALSE );
  			        Main.Step++;
                }
            }
        break;
        
  		case STEP_MAIN_DONE:
            mu8Done = TRUE;
        break;
        
        default:
        break;
    }
    return mu8Done;
}

static U8 DoCleanAfter( DoTypeValue_T *pVal )
{
    U8 mu8Done = FALSE;
    
    mu8Done = DoAfterWithStep( &After.CleanStep, pVal );
    return mu8Done;
}

static U8 DoBidetAfter( DoTypeValue_T *pVal )
{
    U8 mu8Done = FALSE;

    mu8Done = DoAfterWithStep( &After.BidetStep, pVal );
    return mu8Done;
}

static U8 DoAfterWithStep( After_Step_T *pStep, DoTypeValue_T *pVal )
{
    U8 mu8Done = FALSE;

    /*pStep�� After.CleanStep or After.BidetStep */
    switch ( *pStep )
    {
        ////////////////////////////////////////////////////////////////////////////
  		//�ļ�ô
  		////////////////////////////////////////////////////////////////////////////
  		//�ļ�ô: 3way 60�� ���� �� 3way ����
  		case STEP_AFTER_COMEBACK_PRESS:
  		    /*�������� ī��Ʈ ���� �ʱ�ȭ*/
  			Mode.ForeceTimer = 0;
  			  
  			/*Function Complete*/
  			Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
            SetWaterPressUserPress ( FALSE );

            if ( Mode.Change == TRUE )
      		{
      		    Mode.Change = FALSE;
                
      		    /*��->����*/
      		    if ( Mode.Sel == TYPE_CLEAN )
      		    {
        		    SetCleanMode( CLEAN_MODE_ALL );
                    InitCleanAfter();
        		}
        		
      		    /*����->��*/
      		    if ( Mode.Sel == TYPE_BIDET )
      		    {
        		    SetBidetMode( BIDET_MODE_ALL );
                    InitBidetAfter();
        		}
      		}     			
  			(*pStep)++;
  		break;
  
        case STEP_AFTER_COMEBACK_NOZZLE :
  		    if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
  			{
  			    Motor_State( pVal->TypeMotor, 0, pVal->TypeMotorBit );  //����/�� ���� ����
                Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );      //4Way ����

                //2014-10-10 �ŷڼ� ������ ����: �ڵ���� �� �ļ�ô ����
                //�����׽�Ʈ ��� ī��Ʈ
                if ( GetSterAllCount() >= STER_ALL_PERIOD
                    || GetSterReservation() == TRUE
                    || GetWaterLevel() == FALSE )
                {
                    Mode.SterPass = TRUE;
                }
                
  				(*pStep)++;

  			}
  		break;
  
        case STEP_AFTER_STER:
  		    if( IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE 
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE )
  			{
  			    if ( Mode.SterPass == TRUE )
  			    {
  			        Mode.SterPass = FALSE;

			        SetSterAfter( FALSE );
  				}
  				else
  				{
  				    //2015-01-15 ����-��-������ ������忡�� ������ �ļ�ô ���� ����
  				    if( GetSterCountEnable() == TRUE )
  				    {
  				        SetSterAfter( TRUE );
  				    }
  				    else
  				    {
  				        SetSterAfter( FALSE );
  				    }
  				}
  				(*pStep)++;
  			}
  		break;
  
        case STEP_AFTER_END:
  		    if( GetSterAfter() == FALSE )
  			{
  			    if ( pVal->TypeMotor == TYPE_MOTOR_CLEAN )
  				{
  				    /*Clean -> Clean*/
      				if ( Mode.Sel == TYPE_CLEAN )  
      				{
      				    Mode.Sel = TYPE_WAIT;
      				}
      				/*Clean -> Bidet*/
      				else if ( Mode.Sel == TYPE_BIDET )
      				{
      				    Mode.Sel = TYPE_BIDET;
      				}
      				/*Clean -> Dry*/
      				else if ( Mode.Sel == TYPE_DRY)
      				{
      				    Mode.Sel = TYPE_DRY;	          //�� �� ���� �����ϸ� �ļ�ô ��ġ�� ���� ��������
      				}
  				}
  				
  				if ( pVal->TypeMotor == TYPE_MOTOR_BIDET )
  				{
  				    /*Bidet -> Clean*/
      				if ( Mode.Sel == TYPE_CLEAN )
    			    {
    			        Mode.Sel = TYPE_CLEAN;
    			    }
    			    /*Bidet -> Bidet*/
    			    else if( Mode.Sel == TYPE_BIDET)
    			    {
    			        Mode.Sel = TYPE_WAIT;
    			    }
    			    /*Bidet -> Dry*/
    			    else if( Mode.Sel == TYPE_DRY)
    			    {
    			        Mode.Sel = TYPE_DRY;	          //�� �� ���� �����ϸ� �ļ�ô ��ġ�� ���� ��������
    			    }
    			}
  				
  				(*pStep)++;
  			}	
  		break;
  		
  		case STEP_AFTER_DONE:
            mu8Done = TRUE;
        break;
        
        default:
        break;
    }
    return mu8Done;
}





