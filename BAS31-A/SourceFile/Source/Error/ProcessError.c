#include "hw.h"

#include "ProcessError.h"
#include "ErrorCheck.h"
#include "eeprom.h"
#include "Process_display.h"
#include "Adc.h"
#include "Power.h"
#include "Setting.h"
#include "Sterilize.h"
#include "SteppingMotor.h"
#include "Valve.h"
#include "Mode.h"

#include "Aging.h"
#include "Factorymode.h"

Error_T  Err;









void InitError(void)
{
    memset( &Err, 0, sizeof( Error_T ));
}

void GetErrorData(Error_T *pData)
{
    memcpy( pData, &Err, sizeof( Error_T));
}

/*해당 ID Err 여부 판단*/
U8	IsError(ErrorId_T mId )
{
    return Err.Status[ mId ];
}

/* watersupply 제외 Err 여부 판단*/
U8 IsErrorTypeExceptWaterSupply(void)
{
    if( IsError( ERR_SEAT_OVER ) == TRUE 
        || IsError( ERR_WATER_OVER ) == TRUE 
        || IsError( ERR_OUT_SHORT_OPEN ) == TRUE 
        || IsError( ERR_SEAT_SHORT_OPEN ) == TRUE 
        || IsError( ERR_WATER_LEAK ) == TRUE )
    {
        return TRUE;
    }

    return FALSE;
}

/*Stop 제어 필요 Err 여부 판단*/
U8 IsErrorTypeStop(void)
{
    if( IsError( ERR_SEAT_OVER ) == TRUE 
        || IsError( ERR_WATER_OVER ) == TRUE 
        || IsError( ERR_OUT_SHORT_OPEN ) == TRUE 
        || IsError( ERR_SEAT_SHORT_OPEN ) == TRUE 
        || IsError( ERR_WATER_LEAK ) == TRUE 
        || IsError( ERR_WATER_SUPPLY ) == TRUE 
        || IsError( ERR_WATER_SUPPLY_CRITICAL ) == TRUE )
    {
        return TRUE;
    }

    return FALSE;
}

/*Err enum 값 GET*/
ErrorId_T	GetErrorId(void)
{
    return Err.ErrorId;
}

/*Memento enum 값 GET*/
ErrorId_T	GetMementoError(U8 mu8Index)
{
    if( mu8Index < MEMENTO_LIST_NUM )
    {
        return Err.MementoList[ mu8Index ];
    }

    return ERR_NONE;
}

void	SetMementoError(U8 mu8Index, ErrorId_T mError )
{
    Err.MementoList[ mu8Index ] = mError;
}

void ClearMementoError(void)
{
    SetMementoError( 0, ERR_NONE );
    SetMementoError( 1, ERR_NONE );
    SetMementoError( 2, ERR_NONE );
    SetMementoError( 3, ERR_NONE );
    SetMementoError( 4, ERR_NONE );
    SaveEepromId( EEP_ID_MEMENTO_1 );
}





static void UpdateMementoError( ErrorId_T mErr )
{
/* 2018-05-09 김종민 QM. 동일 Err 발생시 저장하지 말것 */
#if 0
        if( mErr != ERR_NONE )
#endif
    if ( mErr != ERR_NONE 
        && mErr != Err.MementoList[ 4 ] )
    {
        Err.MementoList[ 0 ] = Err.MementoList[ 1 ];
        Err.MementoList[ 1 ] = Err.MementoList[ 2 ];
        Err.MementoList[ 2 ] = Err.MementoList[ 3 ];
        Err.MementoList[ 3 ] = Err.MementoList[ 4 ];
        Err.MementoList[ 4 ] = mErr;

        SaveEepromId( EEP_ID_MEMENTO_1 );
    }
}





typedef U8 (*ErrFun_T)(U8 mu8Error);
typedef struct _error_list_
{
    ErrorId_T Id;

    U8 Error;

    ErrFun_T Check;
    ErrFun_T Release;
} ErrorList_T;



ErrorList_T ErrList[ ERR_NUM ] = 
{
    /*ID                            Error   Check error                    Check release error */
    { ERR_SEAT_SHORT_OPEN,          FALSE,  CheckErrSeatShortOpen,         ReleaseErrSeatShortOpen },
    { ERR_OUT_SHORT_OPEN,           FALSE,  CheckErrInOutShortOpen,        ReleaseErrInOutShortOpen },
    { ERR_WATER_OVER,               FALSE,  CheckErrWaterOver,             ReleaseErrWaterOver },
    { ERR_SEAT_OVER,                FALSE,  CheckErrSeatOver,              NULL }, // There is no release error
    { ERR_WATER_LEAK,               FALSE,  CheckErrWaterLeak,             ReleaseErrWaterLeak }, // There is no release error
    { ERR_WATER_SUPPLY,             FALSE,  CheckErrWaterSupply,           ReleaseErrWaterSupply },
    { ERR_WATER_SUPPLY_CRITICAL,    FALSE,  CheckErrWaterSupplyCritical,   ReleaseErrWaterSupplyCritical }
};
#define  SZ_ERR_LIST      (sizeof(ErrList)/sizeof(ErrorList_T))


static void ErrorLoadClear ( void );


/*100msec Timer*/
void Evt_100msec_ProcessError(void)
{
    U8 i;
    ErrFun_T pCheck;
    ErrFun_T pRelease;
    ErrorId_T mId = ERR_NONE;
    ErrorId_T mNewId = ERR_NONE;
    U8 mu8Error = FALSE;

    if ( GetPowerOnOff() == FALSE
        || GetMementoDisp() == TRUE
        || GetADCDelay() != 0 
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        return;
    }
    
    for( i = 0; i < SZ_ERR_LIST; i++ ) 
    {
        pCheck   = ErrList[ i ].Check;
        pRelease = ErrList[ i ].Release;
        mu8Error = ErrList[ i ].Error;

        // 현재 상태가  에러이면, 에러 해제 조건 검사 
        if( mu8Error == TRUE )
        {
            if( pRelease != NULL )
            {
                mu8Error = pRelease( mu8Error );
            }
        }
        // 현재 상태가 정상이면, 에러 발생 조건 감사
        else
        {
            if( pCheck != NULL )
            {
                mu8Error = pCheck( mu8Error );
            }
        }

        // Update new error status
        ErrList[ i ].Error = mu8Error;

        // Mapping error status..
        mId = ErrList[ i ].Id;
        Err.Status[ mId ] = ErrList[ i ].Error;

        //  Top priority Error
        if( Err.Status[ mId ] == TRUE )
        {
            mNewId = mId;
        }
    }

    // Update New Error Id
    if( Err.ErrorId != mNewId ) 
    {
        Err.PrevErrorId = Err.ErrorId;
        UpdateMementoError( mNewId );
    }
    Err.ErrorId = mNewId;

    
    if ( Err.ErrorId != ERR_NONE )
    {
        ErrorLoadClear();
    }
    else
    {
    }
}


static void ErrorLoadClear ( void )
{
    //밸브 제외한 모든 부하 OFF

    /*Dry Heater 관련된 부하는 Stop_Function()에 의해서 처리*/
    //TURN_OFF_DRY_HEATER();
    //TURN_OFF_DRY_RELAY();
    //TURN_OFF_FAN();
    U8 mu8GetFactoryTest_Remote;


    TURN_OFF_WATER_HEATER();
    TURN_OFF_SEAT_HEATER();

    TURN_OFF_AIR_PUMP();

    if( GetAgingStart() == TRUE )
    {
        SetAgingStart ( FALSE );
        SetAgingMode ( AGING_MODE_NONE );
    }

    mu8GetFactoryTest_Remote = GetFactoryTest_Remote();
    if ( mu8GetFactoryTest_Remote == TRUE )
    {
        SetFactoryTest_Remote ( FALSE );
        SetFactoryAutoTest ( FALSE );
        SetFactoryAutoTestStep ( AUTO_TEST_INIT );

        Motor_State ( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
        Motor_State ( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
        Motor_State ( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
        Motor_State ( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );
    }

    /*2016-04-02. 특수설정모드에서 Err 발생시 특수설정모드 해제*/
    SetSettingTime(0);

    if(GetSterAll() == TRUE)
    {
        EW_Stop();
        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
    }

    SetModeFlag ( MODE_FLAG_STOP );
}


