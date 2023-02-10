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
    EEP_ID_SOUND,               // ���� ����          default ON
    EEP_ID_POWER_SAVE,          // ���� ����          default General Saving
    EEP_ID_SEAT_DETECT,         // ���°��� ����      default FALSE -> �����ܼ����� �� TRUE
    EEP_ID_MOOD,                // ����� ����        default FALSE
    EEP_ID_NOZZLE,              // ������ġ           default 3
    EEP_ID_AUTO_DRAIN,          // �ڵ���� ��� 24�ð�
    EEP_ID_EMPTY_0B,            // ����Ŀ�� Down AD
    EEP_ID_EMPTY_0C,            // ����Ŀ�� UP AD
    EEP_ID_EMPTY_0D,            // ������Ʈ Down AD
    EEP_ID_EMPTY_0E,            // ������Ʈ UP AD
    EEP_ID_MEMENTO_1,           // �޸��� ���� 1
    
    //0x10
    EEP_ID_MEMENTO_2,           // �޸��� ���� 2
    EEP_ID_MEMENTO_3,           // �޸��� ���� 3
    EEP_ID_MEMENTO_4,           // �޸��� ���� 4
    EEP_ID_MEMENTO_5,           // �޸��� ���� 5
    EEP_ID_EMPTY_14,
    EEP_ID_BODY_CLEAN,          // �׹�����
    EEP_ID_EMPTY_16,            // ȸ���� ���� ��� ���и�� (Ư������)
    EEP_ID_EMPTY_17,            // ȸ���� ��� �ʱ� ���� Ȯ�� 
    EEP_ID_PRESS,               // ����               default 3
    EEP_ID_SAVE_MODE,           // ����,���� ������ default FALSE

    
    EEP_ID_WATER_TANK_INITIAL,  // �ʱ� ���� �ΰ��� �� ä��� ���� 
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
