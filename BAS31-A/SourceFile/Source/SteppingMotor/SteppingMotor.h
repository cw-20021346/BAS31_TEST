#ifndef __STEPPINGMOTOR_H__
#define __STEPPINGMOTOR_H__

#include "prj_type.h"

/* MOTOR SELECT */
typedef enum
{
    TYPE_MOTOR_CLEAN,
    TYPE_MOTOR_BIDET,
    TYPE_MOTOR_PRESS,
    TYPE_MOTOR_WIDE,
    TYPE_MOTOR_DRY,
    TYPE_MAX,
} Motorid_T;

/* Target Step */
typedef enum
{
    TARGET_STEP_CLEAN,
    TARGET_STEP_BIDET,
    TARGET_STEP_PRESS,
    TARGET_STEP_PRESS_CARE,
    TARGET_STEP_WIDE,
    TARGET_STEP_DRY,
    TARGET_STEP_MAX,

} TargetMotorid_T;


#define MOTOR_STOP                      200


#define HOLE_MAX                 6


#define TYPE_BIT_MOTOR_NONE     0x00
#define TYPE_BIT_MOTOR_CLEAN    0x01
#define TYPE_BIT_MOTOR_BIDET    0x02
#define TYPE_BIT_MOTOR_PRESS    0x04
#define TYPE_BIT_MOTOR_WIDE     0x08
#define TYPE_BIT_MOTOR_DRY      0x10


/*100usec 기준 동작 시간 입력*/
#define TIME_MOTOR_PRESS    30      /*3msec*/
#define TIME_MOTOR_WIDE     30      /*3msec*/
#define TIME_MOTOR_CLEAN    25      /*2.5msec*/
#define TIME_MOTOR_BIDET    25      /*2.5msec*/
#define TIME_MOTOR_DRY      25      /*2.5msec*/








void InitSteppingMotor(void);


void ClearMotorFirstFlag(U8 mu8Event );
U8 IsSetMotorFirstFlag( U8 mu8Event );


void ClearMotorEndFlag(U8 mu8Event );
void SetMotorEndFlag(U8 mu8Event );
U8 IsSetMotorEndFlag( U8 mu8Event );


void ClearMotorUsedFlag(U8 mu8Event );
void SetMotorUsedFlag(U8 mu8Event );
U8 IsSetMotorUsedFlag( U8 mu8Event );







void SetMotorStep ( U8 mu8Type, U8 mu8State );

void SetMotorCleanMoveCount( U16 mu16Val );
U16 GetMotorCleanMoveCount( void );
void SetMotorBidetMoveCount( U16 mu16Val );
U16 GetMotorBidetMoveCount( void );
void SetMotorWideMoveCount( U16 mu16Val );
U16 GetMotorWideMoveCount( void );
void SetMotorDryMoveCount( U16 mu16Val );
U16 GetMotorDryMoveCount( void );


void SetMotorPressMoveCount( U16 mu16Val );
U16 GetMotorPressMoveCount( void );

void SetMotorPressState( U8 mu8val);
U8 GetMotorPressState(void);

void Motor_State( U8 mu8Type, U8 mu83wayState, U8 mu8TypeBit );
void Evt_100msec_Motor_Initial ( void );


/*Timer에 선언*/
/*예) 3msec에 Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_WIDE, (U16) TIME_MOTOR_WIDE, (U8) TYPE_BIT_MOTOR_WIDE );*/
void Evt_Motor_MOVE_Function( U8 mu8Type, U16 mu16Time, U8 mu8TypeBit);





#endif
