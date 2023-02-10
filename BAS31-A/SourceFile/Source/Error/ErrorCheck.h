#ifndef __ERRORCHECK_H__
#define __ERRORCHECK_H__

#include "prj_type.h"


void InitErrorCheck (void);

U8 CheckErrSeatShortOpen(U8 mu8Error);
U8 ReleaseErrSeatShortOpen(U8 mu8Error);

U8 CheckErrInOutShortOpen(U8 mu8Error);
U8 ReleaseErrInOutShortOpen(U8 mu8Error);

U8 CheckErrWaterOver(U8 mu8Error);
U8 ReleaseErrWaterOver(U8 mu8Error);

U8 CheckErrSeatOver(U8 mu8Error);
U8 ReleaseErrSeatOver(U8 mu8Error);

U8 CheckErrWaterLeak(U8 mu8Error);
U8 ReleaseErrWaterLeak(U8 mu8Error);

U8 CheckErrWaterSupply(U8 mu8Error);
U8 ReleaseErrWaterSupply(U8 mu8Error);
U8 CheckErrWaterSupplyCritical(U8 mu8Error);
U8 ReleaseErrWaterSupplyCritical(U8 mu8Error);

void Evt_100msec_ErrorCheck_Handler ( void );


void SetWaterSupplyErrorReleaseKey ( U8 mu8val );
U8 GetWaterSupplyErrorReleaseKey ( void );

#endif
