#ifndef __KEY_HANDLER_H__
#define __KEY_HANDLER_H__

#include "prj_type.h"

/* MULTI KEY */
#define  K_SETTING              ( K_CLEAN | K_STOP )
#define  K_AGING                ( K_CLEAN | K_BIDET )
#define  K_FACTORY_MODE         ( K_POWER | K_NOZZLE_CLEAN )
#define  K_REMOTE_PARING        ( K_SIDE_1| K_SIDE_2 )

void* GetKeyEventList(void);

U8 GetKeyEventListSize(void);

void* GetKeySettingEventList(void);

U8 GetKeySettingEventListSize(void);

void* GetKeyAgingEventList(void);

U8 GetKeyAgingEventListSize(void);


#endif /* __KEY_HANDLER_H__ */
