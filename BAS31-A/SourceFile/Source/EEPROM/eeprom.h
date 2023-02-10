#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "prj_type.h"

typedef enum
{
    /* CONFIG */
    //0x00
    EEP_ID_INIT_FIRST,          // EEP INIT FIRST

    EEP_ID_EMPTY_01,            // ID
    EEP_ID_EMPTY_02,            // Seat Temp
    EEP_ID_EMPTY_03,            // Water Temp
    EEP_ID_EMPTY_04,            // Wide
    EEP_ID_SOUND,               // 무음 설정          default ON
    EEP_ID_POWER_SAVE,          // 절전 설정          default General Saving
    EEP_ID_SEAT_DETECT,         // 착좌감지 설정      default FALSE -> 공장잔수뺴기 후 TRUE
    EEP_ID_MOOD,                // 무드등 설정        default FALSE
    EEP_ID_NOZZLE,              // 노즐위치           default 3
    EEP_ID_AUTO_DRAIN,          // 자동배수 기능 24시간
    EEP_ID_EMPTY_0B,            // 전동커버 Down AD
    EEP_ID_EMPTY_0C,            // 전동커버 UP AD
    EEP_ID_EMPTY_0D,            // 전동시트 Down AD
    EEP_ID_EMPTY_0E,            // 전동시트 UP AD
    EEP_ID_MEMENTO_1,           // 메멘토 에러 1
    
    //0x10
    EEP_ID_MEMENTO_2,           // 메멘토 에러 2
    EEP_ID_MEMENTO_3,           // 메멘토 에러 3
    EEP_ID_MEMENTO_4,           // 메멘토 에러 4
    EEP_ID_MEMENTO_5,           // 메멘토 에러 5
    EEP_ID_EMPTY_14,
    EEP_ID_BODY_CLEAN,          // 항문세정
    EEP_ID_EMPTY_16,            // 회오리 도기 살균 수압모드 (특수설정)
    EEP_ID_EMPTY_17,            // 회오리 모듈 초기 원점 확인 
    EEP_ID_PRESS,               // 수압               default 3
    EEP_ID_SAVE_MODE,           // 노즐,수압 저장모드 default FALSE

    
    EEP_ID_WATER_TANK_INITIAL,  // 초기 전원 인가시 물 채우기 동작 
    EEP_ID_STATE_DEFAULT,       // Basic / Care / Active State Default Save
    EEP_ID_WATERLESS_TEST,
    EEP_ID_EMPTY_1D,
    EEP_ID_EMPTY_1E,
    EEP_ID_EMPTY_1F,
    
    //0x20
    EEP_ID_INIT_END,          // EEPROM INIT END
    EEP_ID_NUM
} EEPROM_ID_T;

#define WATERLESS_EEP_CHECK     0x87

void InitEeprom(void);
U8 GetEepormInit ( void );

void SaveEepromId( EEPROM_ID_T mu8Id );

U8 WriteData(void);
U8 LoadData(void);

void Evt_1sec_ProcessEeprom(void);


U8 EepromByteWrite( U16 mu16Addr, U8 pData );
U8 EepromByteRead( U16 mu16Addr, U8 *pData );


#endif /* __EEPROM_H__ */
