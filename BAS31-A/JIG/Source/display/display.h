#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "prj_type.h"

/* Buffer index - ROW( 1 ~ 16 ) */
#define ROW_0    ( 0x00000001 )
#define ROW_1    ( 0x00000002 )
#define ROW_2    ( 0x00000004 )
#define ROW_3    ( 0x00000008 )
#define ROW_4    ( 0x00000010 )
#define ROW_5    ( 0x00000020 )
#define ROW_6    ( 0x00000040 )
#define ROW_7    ( 0x00000080 )
#define ROW_8    ( 0x00000100 )
#define ROW_9    ( 0x00000200 )
#define ROW_10   ( 0x00000400 ) 
#define ROW_11   ( 0x00000800 )
#define ROW_12   ( 0x00001000 )
#define ROW_13   ( 0x00002000 )
#define ROW_14   ( 0x00004000 )
#define ROW_15   ( 0x00008000 )

/* Buffer index - COM( 1 ~ 5 ) */
#define COM_0     ( 0x00000000 )
#define COM_1     ( 0x00010000 ) 
#define COM_2     ( 0x00020000 ) 
#define COM_3     ( 0x00030000 ) 
#define COM_4     ( 0x00040000 ) 



#define SEG_LEVEL_3          ( COM_0 | ROW_0 )
#define SEG_LEVEL_2          ( COM_0 | ROW_1 )
#define SEG_LEVEL_1          ( COM_0 | ROW_2 )
#define SEG_STATE_ACTIVE     ( COM_0 | ROW_3 )

#define SEG_HOT_GREEN        ( COM_1 | ROW_0 )
#define SEG_SEAT_GREEN       ( COM_1 | ROW_1 )

#define SEG_HOT_RED          ( COM_2 | ROW_0 )
#define SEG_SEAT_RED         ( COM_2 | ROW_1 )

#define SEG_NOZZLE_CLEAN     ( COM_3 | ROW_0 )
#define SEG_STATE_CARE       ( COM_3 | ROW_1 )
#define SEG_STATE_BASIC      ( COM_3 | ROW_2 )


/* gu8SettingLEDBit */
#define SETTING_BIT_MUTE                0x01
#define SETTING_BIT_SEAT                0x02
#define SETTING_BIT_AUTO_DRAIN          0x04
#define SETTING_BIT_STERILIZE_BODY      0x08
#define SETTING_BIT_POWER_SAVE          0x10


void SetDispSetting (U8 mu8val);
U8 GetDispSetting(void);
void ClearDispSetting (U8 mu8val);
U8 IsSetDispSetting (U8 mu8val);


/* Functinos */
void Evt_1msec_LED_Handler(void);

void HAL_SetByteDisplayBuffer( U8 mu8Index, U8 mu8Val );
void HAL_SetBitDisplayBuffer( U32 mu32Led, U8 mu8OnOff );

void TurnOffAllLED(void);
void TurnOnAllLED(void);

void DispVersion(U8 mu8Val);

void CommonBitOnOff( U32 mu32Val, U8 mu8OnOff );

#endif /* __DISPLAY_H__ */
