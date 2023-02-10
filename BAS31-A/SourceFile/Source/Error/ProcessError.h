#ifndef __PROCESSERROR_H__
#define __PROCESSERROR_H__

#include "prj_type.h"
/* Error Id List
 *
 * Top - Low priority for error display 
 * Bottom - Top Priority for error dispaly
 */
typedef enum
{
    ERR_NONE,
    
    ERR_SEAT_SHORT_OPEN,
    ERR_OUT_SHORT_OPEN,
    ERR_WATER_OVER,
    ERR_SEAT_OVER,
    ERR_WATER_LEAK,
    ERR_WATER_SUPPLY,
    ERR_WATER_SUPPLY_CRITICAL,
    
    ERR_NUM              
} ErrorId_T;


/* ERROR TYPE */
#define  MEMENTO_LIST_NUM   5
typedef struct _error_
{
    ErrorId_T PrevErrorId;    /* Previous ERROR STATUS */
    ErrorId_T ErrorId;        /* Current ERROR STATUS */
    U8 Status[ ERR_NUM ];     /* TRUE : ERROR, FALSE : NORMAL */

    /* Memento */
    ErrorId_T MementoList[ MEMENTO_LIST_NUM ];  /* 발생된 에러 */
}Error_T;


void InitError(void);

void GetErrorData(Error_T *pData);

/*해당 ID Err 여부 판단*/
U8	IsError(ErrorId_T mId );

/* watersupply 제외 Err 여부 판단*/
U8 IsErrorTypeExceptWaterSupply(void);

/*Stop 제어 필요 Err 여부 판단*/
U8 IsErrorTypeStop(void);

/*Err enum 값 GET*/
ErrorId_T	GetErrorId(void);

ErrorId_T	GetMementoError(U8 mu8Index);
void SetMementoError(U8 mu8Index, ErrorId_T mError );
void ClearMementoError(void);

/*100msec Timer*/
void Evt_100msec_ProcessError(void);
#endif
