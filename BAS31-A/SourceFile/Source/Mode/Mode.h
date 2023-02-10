#ifndef __MODE_H__
#define __MODE_H__

#include "prj_type.h"

/* Sel */
typedef enum
{
    TYPE_WAIT,
    TYPE_CLEAN,
    TYPE_BIDET,
    TYPE_DRY,

    TYPE_MODE_MAX,
    
} ModeSelId_T;


/* State */
typedef enum
{
    STATE_NONE,
    STATE_BASIC,
    STATE_ACTIVE,
    STATE_CARE,

    STATE_MAX,
    
} ModeStateId_T;

typedef enum
{
    TABLE_CLEAN_NORMAL,
    TABLE_CLEAN_ACTIVE,
    TABLE_CLEAN_CARE,

    TABLE_BIDET_NORMAL,
    TABLE_BIDET_ACTIVE,
    TABLE_BIDET_CARE,

    
} StateId_T;


typedef enum
{
    WIDE_SELF_CLEAN,
    WIDE_CLEAN_1,
    WIDE_CLEAN_1_5,
    WIDE_CLEAN_2,
    WIDE_BIDET_2,
    WIDE_BIDET_1,       
} Wideid_T;

typedef enum
{
    PRESS_SELF_CLEAN,
    PRESS_1,
    PRESS_2,
    PRESS_3,
    PRESS_POTTERY,
} Wideid_T;


/*Mode Flag*/
#define MODE_FLAG_NONE                  0x00
#define MODE_FLAG_CLEAN_NORMAL          0x01
#define MODE_FLAG_CLEAN_CARE            0x02
#define MODE_FLAG_CLEAN_ACTIVE          0x04
#define MODE_FLAG_BIDET_NORMAL          0x08
#define MODE_FLAG_BIDET_CARE            0x10
#define MODE_FLAG_BIDET_ACTIVE          0x20
#define MODE_FLAG_DRY                   0x40
#define MODE_FLAG_STOP                  0x80

#define MODE_FLAG_ALL ( MODE_FLAG_CLEAN_NORMAL | MODE_FLAG_CLEAN_CARE | MODE_FLAG_CLEAN_ACTIVE | MODE_FLAG_BIDET_NORMAL | MODE_FLAG_BIDET_CARE | MODE_FLAG_BIDET_ACTIVE | MODE_FLAG_DRY | MODE_FLAG_STOP)


/*MODE*/
#define CLEAN_MODE_NONE           0x00
#define CLEAN_MODE_BEFORE         0x01
#define CLEAN_MODE_MAIN           0x02
#define CLEAN_MODE_AFTER          0x04
#define CLEAN_MODE_BEFORE_MAIN    ( CLEAN_MODE_BEFORE | CLEAN_MODE_MAIN )
#define CLEAN_MODE_ALL            ( CLEAN_MODE_BEFORE | CLEAN_MODE_MAIN | CLEAN_MODE_AFTER )

#define BIDET_MODE_NONE           0x00
#define BIDET_MODE_BEFORE         0x01
#define BIDET_MODE_MAIN           0x02
#define BIDET_MODE_AFTER          0x04
#define BIDET_MODE_BEFORE_MAIN    ( BIDET_MODE_BEFORE | BIDET_MODE_MAIN )
#define BIDET_MODE_ALL            ( BIDET_MODE_BEFORE | BIDET_MODE_MAIN | BIDET_MODE_AFTER )


#define MAIN_TIME      6000 /*10msec * 100 *60 = 1min*/

typedef enum
{
    STEP_BEFORE_NOZZLE_CHECK,
    STEP_BEFORE_PRE_HEATING,
    STEP_BEFORE_SELF_PRESS_MOVE,
    STEP_BEFORE_SELF_DRAIN_VALVE_ON,
    STEP_BEFORE_SELF_DRAIN_VALVE_OFF,
    STEP_BEFORE_SELF_DRAIN_NOZZLE,
    STEP_BEFORE_READY,
    STEP_BEFORE_DONE,

} Before_Step_T;

typedef struct _do_before_step_
{
    Before_Step_T Step;
} DoBefore_Step_T;

typedef enum
{
    STEP_MAIN_START_SETTING,
    STEP_MAIN_START_ING,
    STEP_MAIN_PRE_SETTING,
    STEP_MAIN_PRE_ING,
    STEP_MAIN_MAIN_SETTING,
    STEP_MAIN_MAIN_ING,
    STEP_MAIN_RINSE_SETTING,
    STEP_MAIN_RINSE_ING,
    STEP_MAIN_DONE,
    
} Main_Step_T;

typedef struct _do_main_step_
{
    Main_Step_T Step;
} DoMain_Step_T;

typedef enum
{
    STEP_AFTER_COMEBACK_PRESS,
    STEP_AFTER_COMEBACK_NOZZLE,
    //STEP_AFTER_COMEBACK_WIDE,
    STEP_AFTER_STER,
    STEP_AFTER_END,
    STEP_AFTER_DONE,
    
} After_Step_T;

typedef struct _do_after_step_
{
    After_Step_T CleanStep;
    After_Step_T BidetStep;
} DoAfter_Step_T;





static void InitBefore(void);
static void InitMain(void);
static void InitCleanAfter(void);
static void InitBidetAfter(void);


void InitMode(void);

void SetModeSel( U8 mu8val);
U8 GetModeSel();

void SetModeState( U8 mu8val);
U8 GetModeState(void);
void SetModeStateDefault( U8 mu8val);
U8 GetModeStateDefault ( void );

    

void SetModeFlag(U8 mu8Mode);
U8 GetModeFlag(void);
void ClearModeFlag(U8 mu8Mode);
U8 IsSetModeFlag(U8 mu8Mode);

void SetModeTime ( U16 mu16val);
U16 GetModeTime();

void SetCleanMode(U8 mu8Mode);
U8 GetCleanMode(void);
void ClearCleanMode(U8 mu8Mode);
U8 IsSetCleanMode(U8 mu8Mode);

void SetBidetMode(U8 mu8Mode);
U8 GetBidetMode(void);
void ClearBidetMode(U8 mu8Mode);
U8 IsSetBidetMode(U8 mu8Mode);

void SetBeforeStep ( Before_Step_T val );  
void SetMainStep ( Main_Step_T val );

Before_Step_T GetBeforeStep(void);
Main_Step_T GetMainStep(void);
After_Step_T GetAfterCleanStep(void);
After_Step_T GetAfterBidetStep(void);





void Evt_10msec_Mode_Step ( void );


#endif
