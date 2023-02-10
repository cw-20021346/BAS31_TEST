#include "eeprom.h"
#include "util.h"
#include "hal_eeprom.h"

#include "Buzzer.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "ProcessError.h"
#include "PowerSave.h"
#include "Seat.h"
#include "Setting.h"
#include "Sterilize.h"
#include "Waterlevel.h"
#include "Watertank.h"
#include "Mode.h"
#include "FactoryMode.h"


#include "Waterless.h"

#define DEFAULT_EEPROM_INIT_FIRST_VAL    0xAA
#define DEFAULT_EEPROM_INIT_END_VAL      0x55



#define EEP_ADDR_OFFSET   0x0000        /* Device - AT24C02 */
//#define EEP_ADDR_OFFSET   0x0040        /* Device - ISL12026 */


typedef struct _eeprom_control_
{
    /* Init / Write  */
    U8 Init;
    U8 Write;

    U16 DelayTime;
    U8 ReadData[ EEP_ID_NUM ];
    U8 WriteData[ EEP_ID_NUM ];
} EEP_Control_T;


EEP_Control_T  Eep;





void InitEeprom(void)
{
    Eep.Init      = FALSE;
    
    /*EEPROM 초기화*/
    if( LoadData() == FALSE )
    {
        WriteData();
    }

    // Write하고 Delay Time은 LoadData() 이후에 실행되어야 한다.
    // LoadData 동작 중에, Set함수들에 대해서 Eep.Write가 TRUE가되기 때문이다.
    // Eep.Write가 TRUE가되면 불필요한 eeprom write 동작이 한 번 실행되는 것을 막기 위해서이다.
    Eep.Write     = FALSE;
    Eep.DelayTime = 0;
}

U8 GetEepormInit ( void )
{
    return Eep.Init;
}





void SaveEepromId( EEPROM_ID_T mu8Id )
{
    if ( GetFactoryTest() == TRUE )
    {
        return;
    }

    if ( GetFactoryTest_Remote() == TRUE )
    {
        return;
    }

    if( mu8Id < EEP_ID_NUM )
    {
        Eep.DelayTime = 1;  // 1sec..
        Eep.Write = TRUE;  
    }

}

U8 EepromByteWrite( U16 mu16Addr, U8 pData )
{

    mu16Addr += EEP_ADDR_OFFSET;
    return HAL_RTC_ByteWrite ( DEV_ADDR_EEP, mu16Addr, pData );
}

U8 EepromByteRead( U16 mu16Addr, U8 *pData )
{
    mu16Addr += EEP_ADDR_OFFSET;
    return HAL_RTC_ByteRead ( DEV_ADDR_EEP, mu16Addr, pData );
}




static U8 EepromPageWrite( U16 mu16Addr, U8 *pData )
{
//    if( IsRtcBatBackUpMode() == TRUE )
//    {
//        return FALSE;
//    }

    mu16Addr += EEP_ADDR_OFFSET;
    return HAL_RTC_PageWrite( DEV_ADDR_EEP, mu16Addr, pData, EEP_PAGE_SIZE);
}

static U8 EepromSeqRead( U16 mu16Addr, U8 *pData, U8 mu16Length )
{
//    if( IsRtcBatBackUpMode() == TRUE )
//    {
//        return FALSE;
//    }

    mu16Addr += EEP_ADDR_OFFSET;
    return HAL_RTC_SeqRead( DEV_ADDR_EEP, mu16Addr, pData, mu16Length );
}

static U8 CompareReadData ( U8 mu8eepid, U8 mu8checkdata )
{
    if ( Eep.ReadData[mu8eepid] == mu8checkdata )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


U8 LoadData(void)
{
    U8 mu8Ret;
    U8 mu8comparedata = FALSE;          /* Read Data 일치..여부..확인..*/

    // Read Data
    mu8Ret = EepromSeqRead( EEP_ID_INIT_FIRST, &Eep.ReadData[0], EEP_ID_NUM );
    if( mu8Ret == FALSE )
    {
        return FALSE ;
    }

    // Check 
    if( Eep.ReadData[ EEP_ID_INIT_FIRST ] != DEFAULT_EEPROM_INIT_FIRST_VAL 
            || Eep.ReadData[ EEP_ID_INIT_END ] != DEFAULT_EEPROM_INIT_END_VAL )
    {

        Eep.Init = FALSE;
        return FALSE;
    }

    Eep.Init = TRUE;

    // Load Data
    SetBuzOnOff ( Eep.ReadData[EEP_ID_SOUND] );
    SetSeatSetting ( Eep.ReadData[EEP_ID_SEAT_DETECT] );
    SetPowerSaveMode ( Eep.ReadData[EEP_ID_POWER_SAVE] );
    SetAutoDrainSetting (Eep.ReadData[EEP_ID_AUTO_DRAIN] );
    SetSterSettingBody ( Eep.ReadData[EEP_ID_BODY_CLEAN] );

    //SetSettingMood ( Eep.ReadData[EEP_ID_MOOD] );
    //SetNozzleLevel ( Eep.ReadData[EEP_ID_NOZZLE] );
    //SetNozzleLevelTemp ( Eep.ReadData[EEP_ID_NOZZLE] );
    SetWaterLevelInitial ( Eep.ReadData[EEP_ID_WATER_TANK_INITIAL] );
    SetModeStateDefault ( Eep.ReadData[EEP_ID_STATE_DEFAULT] );
    SetModeState ( Eep.ReadData[EEP_ID_STATE_DEFAULT] );

    SetMementoError( 0, Eep.ReadData[ EEP_ID_MEMENTO_1 ] );
    SetMementoError( 1, Eep.ReadData[ EEP_ID_MEMENTO_2 ] );
    SetMementoError( 2, Eep.ReadData[ EEP_ID_MEMENTO_3 ] );
    SetMementoError( 3, Eep.ReadData[ EEP_ID_MEMENTO_4 ] );
    SetMementoError( 4, Eep.ReadData[ EEP_ID_MEMENTO_5 ] );


    /* Waterless EEP_ID Check */
    mu8comparedata = CompareReadData ( EEP_ID_WATERLESS_TEST, WATERLESS_EEP_CHECK );
    SetWaterLessSeatReady ( mu8comparedata );



    //SetWaterPressLevel ( Eep.ReadData[EEP_ID_PRESS] );
    //SetWaterPressLevelTemp ( Eep.ReadData[EEP_ID_PRESS] );
    //SetSettingSaveMode ( Eep.ReadData[EEP_ID_SAVE_MODE] );   //SET 되면 수압 노즐 3단으로 변경

    return TRUE;
}

U8 WriteData(void)
{
    // Make Data..
    Eep.WriteData[EEP_ID_INIT_FIRST]         = DEFAULT_EEPROM_INIT_FIRST_VAL;

    Eep.WriteData[EEP_ID_SOUND]              = GetBuzOnOff();               /*default ON */
    Eep.WriteData[EEP_ID_SEAT_DETECT]       = GetSeatSetting();             /*default OFF */
    Eep.WriteData[EEP_ID_POWER_SAVE]        = GetPowerSaveMode();           /*default ON */
    Eep.WriteData[EEP_ID_AUTO_DRAIN]        = GetAutoDrainSetting();        /*default OFF */
    Eep.WriteData[EEP_ID_BODY_CLEAN]        = GetSterSettingBody();         /*default ON */
    Eep.WriteData[EEP_ID_WATER_TANK_INITIAL] = GetWaterLevelInitial();      /*default OFF */
    Eep.WriteData[EEP_ID_STATE_DEFAULT]     = GetModeStateDefault();        /*default BAISC */   
    

    Eep.WriteData[EEP_ID_MEMENTO_1]         = (U8)GetMementoError( 0 );
    Eep.WriteData[EEP_ID_MEMENTO_2]         = (U8)GetMementoError( 1 );
    Eep.WriteData[EEP_ID_MEMENTO_3]         = (U8)GetMementoError( 2 );
    Eep.WriteData[EEP_ID_MEMENTO_4]         = (U8)GetMementoError( 3 );
    Eep.WriteData[EEP_ID_MEMENTO_5]         = (U8)GetMementoError( 4 );
    
        
//    Eep.WriteData[EEP_ID_PRESS]             = GetWaterPressLevel();
//    Eep.WriteData[EEP_ID_NOZZLE]            = GetNozzleLevel();
//    Eep.WriteData[EEP_ID_SAVE_MODE]         = FALSE;    //GetSettingSaveMode();
//    Eep.WriteData[EEP_ID_MOOD]              = GetSettingMood();       



    Eep.WriteData[EEP_ID_INIT_END]           = DEFAULT_EEPROM_INIT_END_VAL;

    // Write data..
    {
        U16 i;
        U8 mu8PageNum;

        mu8PageNum = EEP_ID_NUM + EEP_PAGE_SIZE;
        mu8PageNum /= EEP_PAGE_SIZE;
        for( i = 0; i < mu8PageNum; i++ )
        {
            U16 mu16Index;

            mu16Index = i * EEP_PAGE_SIZE;
            EepromPageWrite( mu16Index, &Eep.WriteData[ mu16Index ] );
        }
    }

    return TRUE;
}

/*Timer 1sec*/
void Evt_1sec_ProcessEeprom(void)
{
    // Write 명령 조건 검사
    if( Eep.Write == FALSE )
    {
        return ;
    }

    // Write 시간 지연 확인
    if( Eep.DelayTime != 0 )
    {
        Eep.DelayTime--;
        return ;
    }

    // Write Data
//    if( IsRtcBatBackUpMode() == FALSE )
//    {
        WriteData();
        Eep.Write = FALSE;
//    }
}

