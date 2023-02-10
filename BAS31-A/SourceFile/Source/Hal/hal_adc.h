#ifndef __HAL_ADC_H__
#define __HAL_ADC_H__

#include "prj_type.h"

/*1msec ¸¶´Ù ADC*/
//void HAL_ProcessAdc(void)


/* ADC ID */
enum
{
    ANI_SEAT_TEMPERATURE,       /* AD0 */
    ANI_OUT_TEMPERATURE,        /* AD1 */
    ANI_IN_TEMPERATURE,         /* AD2 */
    ANI_EW_FEEDBACK,            /* AD3 */
    ANI_VOLTAGE_FEEDBACK,       /* AD4 */
    ANI_VALVE_FEEDBACK,         /* AD5 */

    ANI_MAX_NUM
};


void HAL_InitAdc(void);

U16 HAL_GetAdcValue(U8  mu8Id );


void HAL_ProcessAdc(void);


#endif /* __HAL_ADC_H__ */

