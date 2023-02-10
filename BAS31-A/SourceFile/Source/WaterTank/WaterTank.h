#ifndef __WATERTANK_H__
#define __WATERTANK_H__

#include "prj_type.h"

#define CHECK_CLEAN_NOZZLE      0
#define CLEAN_TIME              1
#define END_CLEAN_NOZZLE        2
#define CHECK_BIDET_NOZZLE      3
#define BIDET_TIME              4
#define END_BIDET_NOZZLE        5


void InitWaterTank ( void );


void SetNozzleClean( U8 mu8val);
U8 GetNozzleClean(void);

void SetNozzleCleanStep( U8 mu8val);
U8 GetNozzleCleanStep(void);

void SetAutoDrainSetting ( U8 mu8Val );
U8 GetAutoDrainSetting ( void );
void ToggleSettingAutoDrain ( void );

void SetAutoDrainStart ( U8 mu8val );
U8 GetAutoDrainStart ( void );
void SetAutoDrainTime ( U16 mu8val );
U16 GetAutoDrainTime ( void );
    

void Evt_100msec_NozzleClean_Handler( void );
void StopNozzleClean ( void );
void Evt_1min_AutoDrain_Handler( void );


#endif
