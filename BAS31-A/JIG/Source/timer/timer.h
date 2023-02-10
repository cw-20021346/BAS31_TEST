#ifndef __TIMER_H__
#define	__TIMER_H__	

#include "timer_id.h"

#define	DELAY_MS(x) 
#define	SEC( x ) 		( ( x ) * 1000UL )

#define SYSTEM_BASETIME_1MS             1
#define SYSTEM_BASETIME_10MS            10
#define SYSTEM_BASETIME_100MS           100








void InitTimer( void );

void StartTimer( U8 id, U32 time_out );

void DisableTimer( U8 id );

void StopTimer( U8 id );

#define	TIMER_EXPIRE		0
#define	TIMER_NOT_EXPIRE	1
#define	TIMER_DISABLE		2
U8 IsExpiredTimer( U8 id );

void RegisterTimerISR( void (*pUserISR)(void) );

void UpdateTimer( void );
#endif /* __TIMER_H__ */
