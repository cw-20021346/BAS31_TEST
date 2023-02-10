#include "hw.h"

#include "display.h"

#define DISPLAY_DATA_RAM_LENGTH  16

/*****************************************************************************
 * WdataArray
 *                        bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0
 * COM7  WdataArray[15] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[14] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM6  WdataArray[13] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[12] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM5  WdataArray[11] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[10] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM4  WdataArray[9] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[8] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM3  WdataArray[7] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[6] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM2  WdataArray[5] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[4] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM1  WdataArray[3] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[2] = row07 row06 row05 row04 row03 row02 row01 row00 
 * COM0  WdataArray[1] = row15 row14 row13 row12 row11 row10 row09 row08 
 *       WdataArray[0] = row07 row06 row05 row04 row03 row02 row01 row00 
 */
U8 WdataArray[ DISPLAY_DATA_RAM_LENGTH ];
U8 gu8Sel = 0;

/* Setting ON/OFF Bit */
/* 1. Mute */
/* 2. Seat */
/* 3. AutoDrain */
/* 4. SterBody */
/* 5. Powersave */
U8 gu8SettingLEDBit = 0xFF;


void SetDispSetting (U8 mu8val)
{
    gu8SettingLEDBit |= mu8val;
}

U8 GetDispSetting(void)
{
    return gu8SettingLEDBit;
}

void ClearDispSetting (U8 mu8val)
{
    gu8SettingLEDBit &= ~mu8val;
}

U8 IsSetDispSetting (U8 mu8val)
{
    if ( ( gu8SettingLEDBit & mu8val ) == mu8val )
    {
        return TRUE;
    }
    
    return FALSE;
}


void Evt_1msec_LED_Handler(void)
{

    /* DELAY TIME
     * LED MATRIX를 스위칭 할때 희미하게 다른 LED들이 켜지는 것을 방지하기 
위함.
     */
    INIT_LED_SCAN();
    INIT_LED_SELECT();

//    NOP(); NOP(); NOP(); NOP(); NOP();
//    NOP(); NOP(); NOP(); NOP(); NOP();
//    NOP(); NOP(); NOP(); NOP(); NOP();
//    NOP(); NOP(); NOP(); NOP(); NOP();

    NOP(); NOP(); NOP(); NOP(); NOP();
    NOP(); NOP(); NOP(); NOP(); NOP();
    NOP(); NOP(); NOP(); NOP(); NOP();
    NOP(); NOP(); NOP(); NOP(); NOP();

    /* SELECT COM */
    switch(gu8Sel)
    {
        case 0 : SCAN_1 = 0; break;
        case 1 : SCAN_2 = 0; break;
        case 2 : SCAN_3 = 0; break;
        case 3 : SCAN_4 = 0; break;
//        case 4 : SCAN_5 = 0; break;
        
        default : SCAN_4 = 0; break;
    }

    /* LED ALL OFF */
        /* SELECT SEG */
    SELECT_1 = ((WdataArray[gu8Sel] & 0x01));
    SELECT_2 = ((WdataArray[gu8Sel] & 0x02)>>1);
    SELECT_3 = ((WdataArray[gu8Sel] & 0x04)>>2);
    SELECT_4 = ((WdataArray[gu8Sel] & 0x08)>>3);

    if(gu8Sel < 3) 
    {
        gu8Sel++;
    }
    else 
    {
        gu8Sel = 0;
    }
} 


void HAL_SetByteDisplayBuffer( U8 mu8Index, U8 mu8Val )
{
    WdataArray[ mu8Index ] = mu8Val;
}


void HAL_SetBitDisplayBuffer( U32 mu32Led, U8 mu8OnOff )
{
    U8 mu8Com;
    U8 mu8Row;


    mu8Com = (U8)((mu32Led & 0x00FF0000) >> 16);
    mu8Row = (U8)(mu32Led & 0x0000FFFF);

    if( mu8OnOff != 0 )
    {
        WdataArray[mu8Com]     |= mu8Row;
    }
    else
    {
        WdataArray[mu8Com]     &= ~mu8Row;
    }
}

void TurnOffAllLED(void)
{
    U8   i;

    for( i = 0; i < DISPLAY_DATA_RAM_LENGTH ; i++ )
    {
        HAL_SetByteDisplayBuffer( i, 0 );
    }



    TURN_OFF_LED_MOOD_BLUE();
    TURN_OFF_LED_MOOD_RED();


}

void TurnOnAllLED(void)
{
    U8 i;

    for( i = 0; i < DISPLAY_DATA_RAM_LENGTH ; i++ )
    {
        HAL_SetByteDisplayBuffer( i, 0xFF );
    }

    TURN_ON_LED_MOOD_BLUE();
    TURN_ON_LED_MOOD_RED();
        
}

void CommonBitOnOff( U32 mu32Val, U8 mu8OnOff )
{
    if( mu8OnOff )
        HAL_SetBitDisplayBuffer( mu32Val, ON );
    else
        HAL_SetBitDisplayBuffer( mu32Val, OFF );

}

void DispVersion(U8 mu8Version)
{
    U8 mu8VersionBit_1;
    U8 mu8VersionBit_2;
    U8 mu8VersionBit_3;
    

    mu8VersionBit_1 = ( mu8Version & 0x01 );
    mu8VersionBit_2 = ( mu8Version & 0x02 );
    mu8VersionBit_3 = ( mu8Version & 0x04 );

    if ( mu8VersionBit_1 == 0x01 )
    {
        CommonBitOnOff( SEG_LEVEL_1, ON );
    }
    else
    {
        CommonBitOnOff( SEG_LEVEL_1, OFF );
    }
    
    if ( mu8VersionBit_2 == 0x02 )
    {
        CommonBitOnOff( SEG_LEVEL_2, ON );
    }
    else
    {
        CommonBitOnOff( SEG_LEVEL_2, OFF );
    }

    if ( mu8VersionBit_3 == 0x04 )
    {
        CommonBitOnOff( SEG_LEVEL_3, ON );
    }
    else
    {
        CommonBitOnOff( SEG_LEVEL_3, OFF );
    }
}

