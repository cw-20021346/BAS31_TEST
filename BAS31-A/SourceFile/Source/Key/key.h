#ifndef __KEY_H__
#define __KEY_H__

#include "hal_key.h"


void InitKey(void);
U32 GetKeyVal( void );
U32 GetKeyPopVal( void );


/* EvnetFlag Index */
#define  KEY_EVENT_PUSHED_SHORT_ON  0x01
#define  KEY_EVENT_PUSHED_LONG_5S   0x02
#define  KEY_EVENT_PUSHED_LONG_3S   0x04
#define  KEY_EVENT_PUSHED_LONG_2S   0x08
#define  KEY_EVENT_POP              0x10
#define  KEY_EVENT_PUSHED_SHORT_OFF 0x20
#define  KEY_EVENT_ALL              0xFF



void SetKeyEventFlag(U8 mu8Event );
void ClearKeyEventFlag(U8 mu8Event );
U8 IsSetKeyEventFlag( U8 mu8Event );


void Evt_1msec_ProcessScanKey( void );

#endif /* __KEY_H__ */
