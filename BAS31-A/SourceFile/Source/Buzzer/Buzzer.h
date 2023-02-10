#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "prj_type.h"

typedef enum
{
    BUZZER_NONE,

    BUZZER_POWER_ON,                /* 전원 ON */                                       
    BUZZER_POWER_OFF,               /* 전원 OFF */                                      
    BUZZER_SELECT,                  /* 일반버튼 입력 시 */                              
    BUZZER_SET_ON,                  /* 부가 기능 (어린이,에이징모드,공장잔수빼기) ON */ 
    BUZZER_SET_OFF,                 /* 부가 기능 (어린이,에이징모드,공장잔수빼기) OFF */
    BUZZER_WARNING,                 /* 경고음 */   
    BUZZER_NOZZLE_CLEAN,
    BUZZER_MEMENTO_1,               /* memento 1 */
    BUZZER_MEMENTO_2,               /* memento 2 */
    BUZZER_MEMENTO_3,               /* memento 3 */
    BUZZER_MEMENTO_4,               /* memento 4 */
    BUZZER_MEMENTO_5,               /* memento 5 */

    
    //BUZZER_DRY,                     /* 건조버튼 입력 시 */                              
    //BUZZER_BIDET,                   /* 비데버튼 입력 시 */                              
    //BUZZER_CLEAN,                   /* 세정버튼 입력 시 */                              
    //BUZZER_SELECT,                  /* 일반버튼 입력 시 */                              
    //BUZZER_POWER_ON,                /* 전원 ON */                                       
    //BUZZER_POWER_OFF,               /* 전원 OFF */                                      
    //BUZZER_AIRPLUS_ON,              /* AirPlus ON */                                    
    //BUZZER_SET_OFF,                 /* 부가 기능 (어린이,에이징모드,공장잔수빼기) OFF */
    //BUZZER_SET_ON,                  /* 부가 기능 (어린이,에이징모드,공장잔수빼기) ON */ 
    //BUZZER_MANUAL_STER_START,       /* 수동 살균 시작음*/                               
    //BUZZER_MANUAL_STER_END,         /* 수동 살균 완료음*/                               
    //BUZZER_GREENPLUG_ON,            /* 그림플러그 ON */                                 
    //BUZZER_GREENPLUG_OFF,           /* 그림플러그 OFF */                                
    //BUZZER_SPECIAL_MODE,            /* 특수모드 진입음 */                               
    //BUZZER_WARNING,                 /* 경고음 */                                        
    //BUZZER_NOZZLE_CLEAN,            /* 노즐 살균 시작음 */                              
    //BUZZER_AIRPLUS_OFF,             /* AirPlus OFF*/                                    
    //BUZZER_MEMENTO_1,               /* memento 1 */
    //BUZZER_MEMENTO_2,               /* memento 2 */
    //BUZZER_MEMENTO_3,               /* memento 3 */
    //BUZZER_MEMENTO_4,               /* memento 4 */
    //BUZZER_MEMENTO_5,               /* memento 5 */
    
    BUZZER_MAX
} BuzzerId_T;



/* 부 저 설정 */
//#define BUZZER_DRY                 1		/* 건조버튼 입력 시 */
//#define BUZZER_BIDET               2		/* 비데버튼 입력 시 */
//#define BUZZER_CLEAN               3		/* 세정버튼 입력 시 */
//#define BUZZER_SELECT              4		/* 일반버튼 입력 시 */
//#define BUZZER_POWER_ON            5		/* 전원 ON */
//#define BUZZER_POWER_OFF           6		/* 전원 OFF */
//#define BUZZER_AIRPLUS_ON          7		/* AirPlus ON */
//#define BUZZER_SET_OFF             8		/* 부가 기능 (어린이,에이징모드,공장잔수빼기) OFF */
//#define BUZZER_SET_ON              9		/* 부가 기능 (어린이,에이징모드,공장잔수빼기) ON */
//#define BUZZER_MANUAL_STER_START   10		/* 수동 살균 시작음*/
//#define BUZZER_MEMENTO             11		/* 메멘토 */
//#define BUZZER_MANUAL_STER_END     12		/* 수동 살균 완료음*/
//#define BUZZER_GREENPLUG_ON        13		/* 그림플러그 ON */
//#define BUZZER_GREENPLUG_OFF       14		/* 그림플러그 OFF */
//#define BUZZER_SPECIAL_MODE        15		/* 특수모드 진입음 */
//#define BUZZER_WARNING             16		/* 경고음 */
//#define BUZZER_NOZZLE_CLEAN        17		/* 노즐 살균 시작음 */
//#define BUZZER_AIRPLUS_OFF         18		/* AirPlus OFF*/

//#define BUZZER_MEMENTO_1            13		/* 메멘토 1 */
//#define BUZZER_MEMENTO_2            14		/* 메멘토 2 */
//#define BUZZER_MEMENTO_3            15		/* 메멘토 3 */
//#define BUZZER_MEMENTO_4            16		/* 메멘토 4 */
//#define BUZZER_MEMENTO_5            17		/* 메멘토 5 */
//#define BUZZER_MUTE                 18      /* 부저음 셋업 전용 */
//#define BUZZER_NONE					   19		/* 음 종료 */

/* 부저 초기화 */
void	InitBuzzer(void);

/* 부저 무음 */
U8		GetBuzOnOff(void);
void	SetBuzOnOff(U8 mu8Mute );
void    ToggleBuzOnOff ( void );

void	BUZZER_STEP(U8 mu8Step);

/* 부저 타이머 10ms */
void    Evt_10msec_BuzTimeCounter(void);

void	Evt_BuzControl(void);

#endif /* _FUNC_BUZZER_H_ */
