#ifndef __STERILIZE_H__
#define __STERILIZE_H__

#include "prj_type.h"

typedef enum
{
    FLOW_PATH_INITIAL,
    FLOW_PATH_START,
    FLOW_PATH_SELF_CLEAN,
    FLOW_PATH_MOTOR_MOVE, 
    FLOW_PATH_CLEAN_1,
    FLOW_PATH_CLEAN_2,
    FLOW_PATH_BIDET_1,
    FLOW_PATH_BIDET_2,
    FLOW_PATH_ORIGIN_1,
    FLOW_PATH_ORIGIN_2,
    FLOW_PATH_ORIGIN_3,
    FLOW_PATH_FINISH,

    NOZZLE_VALVE_ON,
    NOZZLE_EW_ON,
    NOZZLE_FINISH,

    POTTERY_INITIAL,
    POTTERY_PUMP_ON,
    POTTERY_VALVE_ON,
    POTTERY_EW_ON,
    POTTERY_EW_OFF,
    POTTERY_VALVE_OFF,
    POTTERY_FINISH,
    
} SterAll_T;


#define     STER_ALL_PERIOD             12

#define STERILIZE_BODY_START_TIME       20
#define STERILIZE_BODY_RINSE_TIME       10


void InitSter ( void );

void SetSterOnOff ( U8 mu8Val );
U8 GetSterOnOff ( void );

void SetSterBody ( U8 mu8Val );
U8 GetSterBody ( void );

void SetSterAfter ( U8 mu8Val );
U8 GetSterAfter ( void );

void SetSterAll ( U8 mu8Val );
U8 GetSterAll ( void );

void SetSterAllStep ( U8 mu8Val );
U8 GetSterAllStep ( void );


void SetSterAllCount ( U8 mu8Val );
U8 GetSterAllCount ( void );

void SetSterCountEnable ( U8 mu8Val );
U8 GetSterCountEnable ( void );

void SetSterSettingBody ( U8 mu8Val );
U8 GetSterSettingBody ( void );
void ToggleSterSettingBody ( void );

void SetSterReservation ( U8 mu8Val );
U8 GetSterReservation ( void );


U16 GetSterDisplayFinishBlinkTime ( void );
U16 GetSterDisplayFinishSeatTime ( void );

void SterAllStart( void );
void SterBodyStart( U16 mu16Time );

void EW_Stop( void );
void Evt_100msec_Sterilize_Handler ( void );




#endif
