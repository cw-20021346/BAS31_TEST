#ifndef __KEY_COMMON_HANDLER_H__
#define __KEY_COMMON_HANDLER_H__

#include "prj_type.h"
#include "hal_key.h"



/* SINGLE KEY */
                            /* ÇØ´ç Å° ¸ÅÄª */
#define  K_NONE             HAL_KEY_NONE
#define  K_NOZZLE_CLEAN     HAL_KEY_1
#define  K_POWER            HAL_KEY_2
#define  K_WATER_TEMP       HAL_KEY_3
#define  K_SEAT_TEMP        HAL_KEY_4
#define  K_STER             HAL_KEY_5
#define  K_DRY              HAL_KEY_6
#define  K_NOZZLE_DOWN      HAL_KEY_7
#define  K_PRESS_DOWN       HAL_KEY_8
#define  K_NOZZLE_UP        HAL_KEY_9
#define  K_PRESS_UP         HAL_KEY_10
#define  K_STATE            HAL_KEY_11
#define  K_BIDET            HAL_KEY_12
#define  K_CLEAN            HAL_KEY_13
#define  K_STOP             HAL_KEY_14

#define  K_SIDE_1           HAL_KEY_SIDE_1
#define  K_SIDE_2           HAL_KEY_SIDE_2
#define  K_SIDE_3           HAL_KEY_SIDE_3




/* Not used */
#define  K_MOOD             HAL_KEY_15
#define  K_POWER_SAVE       HAL_KEY_16
#define  K_CHILD            HAL_KEY_17
#define  K_MOVE             HAL_KEY_18
#define  K_AIR              HAL_KEY_19


typedef U8(*KeyFun_T)(void);
typedef struct _key_event_
{
    U32        Key;
    KeyFun_T   actionEvent_1;   // short Power on
    KeyFun_T   actionEvent_2;   // long 3sec
    KeyFun_T   actionEvent_3;   // long 5se
    KeyFun_T   actionEvent_4;   // pop
    KeyFun_T   actionEvent_5;   // Short Power off
} KeyEventList_T;

#endif /* __KEY_COMMON_HANDLER_H__ */
