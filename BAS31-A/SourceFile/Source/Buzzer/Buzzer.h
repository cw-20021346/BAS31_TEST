#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "prj_type.h"

typedef enum
{
    BUZZER_NONE,

    BUZZER_POWER_ON,                /* ���� ON */                                       
    BUZZER_POWER_OFF,               /* ���� OFF */                                      
    BUZZER_SELECT,                  /* �Ϲݹ�ư �Է� �� */                              
    BUZZER_SET_ON,                  /* �ΰ� ��� (���,����¡���,�����ܼ�����) ON */ 
    BUZZER_SET_OFF,                 /* �ΰ� ��� (���,����¡���,�����ܼ�����) OFF */
    BUZZER_WARNING,                 /* ����� */   
    BUZZER_NOZZLE_CLEAN,
    BUZZER_MEMENTO_1,               /* memento 1 */
    BUZZER_MEMENTO_2,               /* memento 2 */
    BUZZER_MEMENTO_3,               /* memento 3 */
    BUZZER_MEMENTO_4,               /* memento 4 */
    BUZZER_MEMENTO_5,               /* memento 5 */

    
    //BUZZER_DRY,                     /* ������ư �Է� �� */                              
    //BUZZER_BIDET,                   /* �񵥹�ư �Է� �� */                              
    //BUZZER_CLEAN,                   /* ������ư �Է� �� */                              
    //BUZZER_SELECT,                  /* �Ϲݹ�ư �Է� �� */                              
    //BUZZER_POWER_ON,                /* ���� ON */                                       
    //BUZZER_POWER_OFF,               /* ���� OFF */                                      
    //BUZZER_AIRPLUS_ON,              /* AirPlus ON */                                    
    //BUZZER_SET_OFF,                 /* �ΰ� ��� (���,����¡���,�����ܼ�����) OFF */
    //BUZZER_SET_ON,                  /* �ΰ� ��� (���,����¡���,�����ܼ�����) ON */ 
    //BUZZER_MANUAL_STER_START,       /* ���� ��� ������*/                               
    //BUZZER_MANUAL_STER_END,         /* ���� ��� �Ϸ���*/                               
    //BUZZER_GREENPLUG_ON,            /* �׸��÷��� ON */                                 
    //BUZZER_GREENPLUG_OFF,           /* �׸��÷��� OFF */                                
    //BUZZER_SPECIAL_MODE,            /* Ư����� ������ */                               
    //BUZZER_WARNING,                 /* ����� */                                        
    //BUZZER_NOZZLE_CLEAN,            /* ���� ��� ������ */                              
    //BUZZER_AIRPLUS_OFF,             /* AirPlus OFF*/                                    
    //BUZZER_MEMENTO_1,               /* memento 1 */
    //BUZZER_MEMENTO_2,               /* memento 2 */
    //BUZZER_MEMENTO_3,               /* memento 3 */
    //BUZZER_MEMENTO_4,               /* memento 4 */
    //BUZZER_MEMENTO_5,               /* memento 5 */
    
    BUZZER_MAX
} BuzzerId_T;



/* �� �� ���� */
//#define BUZZER_DRY                 1		/* ������ư �Է� �� */
//#define BUZZER_BIDET               2		/* �񵥹�ư �Է� �� */
//#define BUZZER_CLEAN               3		/* ������ư �Է� �� */
//#define BUZZER_SELECT              4		/* �Ϲݹ�ư �Է� �� */
//#define BUZZER_POWER_ON            5		/* ���� ON */
//#define BUZZER_POWER_OFF           6		/* ���� OFF */
//#define BUZZER_AIRPLUS_ON          7		/* AirPlus ON */
//#define BUZZER_SET_OFF             8		/* �ΰ� ��� (���,����¡���,�����ܼ�����) OFF */
//#define BUZZER_SET_ON              9		/* �ΰ� ��� (���,����¡���,�����ܼ�����) ON */
//#define BUZZER_MANUAL_STER_START   10		/* ���� ��� ������*/
//#define BUZZER_MEMENTO             11		/* �޸��� */
//#define BUZZER_MANUAL_STER_END     12		/* ���� ��� �Ϸ���*/
//#define BUZZER_GREENPLUG_ON        13		/* �׸��÷��� ON */
//#define BUZZER_GREENPLUG_OFF       14		/* �׸��÷��� OFF */
//#define BUZZER_SPECIAL_MODE        15		/* Ư����� ������ */
//#define BUZZER_WARNING             16		/* ����� */
//#define BUZZER_NOZZLE_CLEAN        17		/* ���� ��� ������ */
//#define BUZZER_AIRPLUS_OFF         18		/* AirPlus OFF*/

//#define BUZZER_MEMENTO_1            13		/* �޸��� 1 */
//#define BUZZER_MEMENTO_2            14		/* �޸��� 2 */
//#define BUZZER_MEMENTO_3            15		/* �޸��� 3 */
//#define BUZZER_MEMENTO_4            16		/* �޸��� 4 */
//#define BUZZER_MEMENTO_5            17		/* �޸��� 5 */
//#define BUZZER_MUTE                 18      /* ������ �¾� ���� */
//#define BUZZER_NONE					   19		/* �� ���� */

/* ���� �ʱ�ȭ */
void	InitBuzzer(void);

/* ���� ���� */
U8		GetBuzOnOff(void);
void	SetBuzOnOff(U8 mu8Mute );
void    ToggleBuzOnOff ( void );

void	BUZZER_STEP(U8 mu8Step);

/* ���� Ÿ�̸� 10ms */
void    Evt_10msec_BuzTimeCounter(void);

void	Evt_BuzControl(void);

#endif /* _FUNC_BUZZER_H_ */
