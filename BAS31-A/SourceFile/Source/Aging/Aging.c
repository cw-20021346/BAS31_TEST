#include "hw.h"

#include "Aging.h"
#include "Mode.h"
#include "Dry.h"
#include "Sterilize.h"
#include "Buzzer.h"
#include "Steppingmotor.h"

typedef struct _Aging_
{
    U8 Start;
    U8 Mode;
    U16 Time;       /* Aging Mode ���� ���� �ð� */

    U8 SterFlag;        /* Aging Repeat Mode���� Ster ���� ���� Ȯ�� flag */

} Aging_T;

Aging_T Aging;

void InitAging ( void )
{
    Aging.Start = FALSE;
    Aging.Mode = AGING_MODE_NONE;
    Aging.Time = 0;

    Aging.SterFlag = FALSE;
}

void SetAgingStart ( U8 mu8Val)
{
    Aging.Start = mu8Val;
}

U8 GetAgingStart ( void )
{
    return Aging.Start;
}

void SetAgingMode ( U8 mu8Val)
{
    Aging.Mode = mu8Val;
}

U8 GetAgingMode ( void )
{
    return Aging.Mode;
}

void SetAgingTime ( U16 mu16Val)
{
    Aging.Time = mu16Val;
}

U16 GetAgingTime ( void )
{
    return Aging.Time;
}


void Evt_100msec_AgingMode ( void )
{

    /* �ʱ� ���� ���� �Ϸ�� aging ���� ���� �ð� 5sec */
    if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        SetAgingTime ( AGING_TIME );       /* Aging Mode ���� ���� �ð� */
    }

    if ( Aging.Time != 0 )
    {
        Aging.Time--;
    }

    
    //������ư ������ ���ӵ����Ҷ�  
    if ( Aging.Mode == AGING_MODE_CONTINUE )   
    {
        if ( GetModeSel() == TYPE_CLEAN
            && IsSetCleanMode ( CLEAN_MODE_MAIN ) == TRUE 
            && GetMainStep() == STEP_MAIN_MAIN_ING )
        {
            SetModeTime( MAIN_TIME );
        }
    }
  
    //�� ��ư ������ ���� �����Ҷ�
    if (Aging.Mode == AGING_MODE_REPEAT )
    {
        //����¡��忡�� ���� ������ ��
        if ( GetModeSel() == TYPE_CLEAN )
        {
            if ( ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == FALSE ) 
                && ( IsSetCleanMode( CLEAN_MODE_MAIN ) == FALSE )
                && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE ) )
            {
                SetModeFlag ( MODE_FLAG_BIDET_ACTIVE );
            }
        }
        //����¡��忡�� �� ������ ����
        else if ( GetModeSel() == TYPE_BIDET )
        {
            if ( ( IsSetBidetMode( BIDET_MODE_BEFORE ) == FALSE ) 
                && ( IsSetBidetMode( BIDET_MODE_MAIN ) == FALSE )
                && ( IsSetBidetMode( BIDET_MODE_AFTER ) == TRUE ) )
            {
                SetModeFlag ( MODE_FLAG_DRY );
            }
        }
        else if ( GetModeSel() == TYPE_DRY )
        {
            /* ���� ���� ���� */
        }
        else
        {
            //����¡��忡�� ���� ������ ���
            if ( GetSterAll() == FALSE )
            {
                if ( Aging.SterFlag == FALSE )
                {
                    Aging.SterFlag = TRUE;
	                SterAllStart();
                }
                else
                {
                    Aging.SterFlag = FALSE;
                    SetModeFlag ( MODE_FLAG_CLEAN_ACTIVE );
                }
            }
            else
            {
                /* ��յ��� ���� */
            }
        }
    }
}

