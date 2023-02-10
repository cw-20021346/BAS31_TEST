#ifndef _REMOTE_H_
#define _REMOTE_H_

void InitRemote ( void );

void SetRemoteParingStart ( U8 mu8val );
U8 GetRemoteParingStart ( void );
    
void INT_Remote(void);
void Evt_100usec_Remote_Handler ( void );
void Evt_1msec_Remote_Handler ( void );
void Evt_100msec_Remote_Pairing_Mode_Handler ( void );

#endif
