#include "mcu.h"
#include "util.h"
#include "hw.h"

I16 GetMin( I16 x, I16 y )
{
	if( x > y )
	{
		return y;
	}

	return x;
}

I16	GetMax( I16 x, I16 y )
{
	if( x > y )
	{
		return x;
	}

	return y;
}

/* 0 ~ 99 */
U8 Hex2Dec(U8 mu8Hex)
{
    U8 mu8Dec;

    mu8Dec = (mu8Hex & 0x0F);
    mu8Dec += ((mu8Hex & 0xF0) >> 4) * 10;;
    
    return mu8Dec;
}

/* 0 ~ 99 */
U8 Dec2Hex(U8 mu8Dec)
{
    U8 mu8Hex;

    mu8Hex = mu8Dec % 10;
    mu8Hex += (mu8Dec / 10) * 16;
    
    return mu8Hex;
}

void	Delay_US( U8 us )
{
	while( us-- )
	{
		NOP();
		NOP();
		NOP();
	}
}

U16	ByteOrderConv( U16 _src )
{
	U16 h_to_l = _src >> 8;
	U16 l_to_h = ( _src & 0x00FF ) << 8;
	
	return ( l_to_h | h_to_l );
}

void	Delay_MS( U16 ms )
{
	U16	i;
	U8		j;
	
	for( i = 0 ; i < ms ; i++ )
	{
		for( j = 0; j < 4 ; j++ )	// 1ms loop delay
		{
			Delay_US( 250 );
			R_WDT_Restart();
		}
	}
}
