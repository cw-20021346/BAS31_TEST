#include "hw.h"
#include "hal_key.h"


#define  HAL_KEY_PUSHED     TRUE
#define  HAL_KEY_POP        FALSE

#if 0
#define  HAL_LEVER_PUSHED   LOW
#endif

U32 gu32KeyVal;

void HAL_GetScanKey( void )
{

#if CONFIG_REMOTE
        /* Side Key */
        if( GET_STATUS_SIDE_KEY_1() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_SIDE_1;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_SIDE_1;
        }
    
        if( GET_STATUS_SIDE_KEY_2() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_SIDE_2;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_SIDE_2;
        }
        
        if( GET_STATUS_SIDE_KEY_3() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_SIDE_3;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_SIDE_3;
        }
     
#if 0
        if( GET_STATUS_SIDE_KEY_4() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_SIDE_4;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_SIDE_4;
        }
#endif

#else
    if ( SCAN_1 == 0 )
    {
        if( GET_STATUS_KEY_1() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_1;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_1;
        }
        
        if( GET_STATUS_KEY_2() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_2;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_2;
        }
        
        if( GET_STATUS_KEY_3() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_3;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_3;
        }
        
        if( GET_STATUS_KEY_4() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_4;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_4;
        }
    }
    else if ( SCAN_2 == 0 )
    {
        if( GET_STATUS_KEY_1() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_5;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_5;
        }
        
        if( GET_STATUS_KEY_2() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_6;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_6;
        }
        
        if( GET_STATUS_KEY_3() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_7;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_7;
        }
        
        if( GET_STATUS_KEY_4() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_8;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_8;
        }
    }
    else if ( SCAN_3 == 0 )
    {
        if( GET_STATUS_KEY_1()== HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_9;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_9;
        }
        
        if( GET_STATUS_KEY_2() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_10;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_10;
        }
        
        if( GET_STATUS_KEY_3() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_11;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_11;
        }
        
        if( GET_STATUS_KEY_4() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_12;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_12;
        }
    }
    else //if ( SCAN_4 == 0 )
    {
        if( GET_STATUS_KEY_1() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_13;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_13;
        }
        
        if( GET_STATUS_KEY_2() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_14;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_14;
        }
        
        if( GET_STATUS_KEY_3() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_15;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_15;
        }
        
        if( GET_STATUS_KEY_4() == HAL_KEY_PUSHED )
        {
            gu32KeyVal |= HAL_KEY_16;
        }
        else
        {
            gu32KeyVal &= ~HAL_KEY_16;
        }
    }
#endif
}

U32 HAL_GetKeyVal( void )
{
    return gu32KeyVal;
}

#if 0
U16 HAL_GetKeyVal(void)
{
    U16  mu16Val = 0;

    if( P_KEY_1 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_1;
    }
    if( P_KEY_2 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_2;
    }
    if( P_KEY_3 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_3;
    }
    if( P_KEY_4 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_4;
    }
    if( P_KEY_5 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_5;
    }
    if( P_KEY_6 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_6;
    }
    if( P_KEY_7 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_7;
    }
    if( P_KEY_8 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_8;
    }
    if( P_KEY_9 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_9;
    }
    if( P_KEY_10 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_10;
    }
    if( P_KEY_11 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_11;
    }
    if( P_KEY_12 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_12;
    }
    if( P_KEY_13 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_13;
    }
    if( P_KEY_14 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_14;
    }
    if( P_KEY_15 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_15;
    }
    if( P_KEY_16 == HAL_KEY_PUSHED )
    {
        mu16Val |= HAL_KEY_16;
    }

    return mu16Val;
}
#endif



#if 0
U8 HAL_GetLeverIce(void)
{
    U8 mu8Val = 0;

    if( GET_MICRO_SW_ICE_LEVER() == HAL_LEVER_PUSHED )
    {
        mu8Val = 0x01;
    }

    return mu8Val;
}

U8 HAL_GetLeverWater(void)
{
    U8 mu8Val = 0;

    if( GET_MICRO_SW_WATER_LEVER() == HAL_LEVER_PUSHED )
    {
        mu8Val = 0x01;
    }

    return mu8Val;
}
#endif
