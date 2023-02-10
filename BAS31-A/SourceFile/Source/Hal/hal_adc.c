#include "hw.h"
#include "hal_adc.h"


/* MACRO ADC */
#define	ADC_SET_CHANNEL(x)		do{ ADS = (x); }while(0)
#define	ADC_START_CONVERSION()	do{ ADCS = 1; }while(0)
#define	ADC_STOP_CONVERSION()	do{ ADCS = 0; }while(0)
#define	ADC_SAMPLING()			   while(!ADIF)
#define	ADC_GET_VALUE()			(ADCR >> 6)

/**
 * 평균 내는 값에서 최대 값과 최소 값은 제외하기 때문에
 * 평균 내고자 하는 횟수에서 2번을 더한 값을 카운터 횟수에 설정해야한다. 
 */

typedef struct _adc_config_
{
    U8	 channel;
    U8    count;     // 더하기 횟수
    U16   min;       // 최대 값 제외
    U16   max;       // 최소 값 제외 변수 
} TAdcConfig;

static const TAdcConfig	adcConfList[ ANI_MAX_NUM ] = 
{ 
    /* ADC CHANNEL,              SUM COUNTER,   MAX VALUE, MIN VALUE */
    { ANI_SEAT_TEMPERATURE,           52U,        1024U,      0U },   
    { ANI_IN_TEMPERATURE,             52U,        1024U,      0U },
    { ANI_OUT_TEMPERATURE,            52U,        1024U,      0U },
    { ANI_EW_FEEDBACK,                52U,        1024U,      0U },
    { ANI_VOLTAGE_FEEDBACK,           52U,        1024U,      0U },
    { ANI_VALVE_FEEDBACK,             52U,        1024U,      0U }
}; 


typedef struct _sensor_
{
    U16  sum;        // ADC 총 합
    U8   sum_count;  // ADC sum한 횟수

    U16  average;    // ADC 평균 값

    U16  min;        
    U16  max;       
} sensor_t;
sensor_t		adcValue[ ANI_MAX_NUM ];



static U16 SampleAdc( U8 channel );
static void	Sum( void );
static void	Average( void );


void HAL_InitAdc(void)
{
    U8   i;

    // 초기화 값을 설정한다.
    for( i = 0; i < ANI_MAX_NUM ; i++ )
    {
        adcValue[ i ].average = SampleAdc( i );
    }
}

U16	HAL_GetAdcValue( U8 mu8Id )
{
    if( mu8Id >= ANI_MAX_NUM )
    {
        return 0;
    }

    return adcValue[ mu8Id ].average;
}



void HAL_ProcessAdc(void)
{
    Sum();
    Average();
}

static U16 SampleAdc( U8 channel )
{
    U16 	raw = 0;


    /* Stop */
    ADIF = 0;
    ADC_STOP_CONVERSION();

    /* Set channel */
    ADC_SET_CHANNEL( channel );

    /* Start adc */
    R_ADC_Set_OperationOn();
    ADC_START_CONVERSION();

    /* Drop garbage value */
    ADC_SAMPLING();
    raw = ADC_GET_VALUE();

    /* Get real value */
    ADC_SAMPLING();
    raw = ADC_GET_VALUE();

    /* Stop */
    ADC_STOP_CONVERSION();
    R_ADC_Set_OperationOff();


    return raw;
}

static void	Sum( void )
{
    U8	i;
    U8	mu8Ch;
    U8   mu8Count;
    U16  mu16Val;

    for( i = 0; i < ANI_MAX_NUM ; i++ )
    {
        mu8Count = adcValue[ i ].sum_count;
        if( mu8Count < adcConfList [ i ].count )
        {
            mu8Ch 	= adcConfList[ i ].channel;
            mu16Val     = SampleAdc( mu8Ch ); 

            adcValue[ i ].sum += mu16Val;
            adcValue[ i ].sum_count++;

            // Sampling min/max value
            if( mu16Val < adcValue[ i ].min )
            {
                adcValue[ i ].min = mu16Val;
            }

            if( mu16Val > adcValue[ i ].max )
            {
                adcValue[ i ].max = mu16Val;
            }
        }
    }
}

static void	Average( void )
{
    U8   i;
    U8   count;
    U16	average;


    for( i = 0; i < ANI_MAX_NUM ; i++ )
    {
        count = adcValue[ i ].sum_count;
        if(  adcConfList[ i ].count == count )
        {
            adcValue[ i ].sum = adcValue[ i ].sum - ( adcValue[ i ].min + adcValue[ i ].max );
            average	= adcValue[ i ].sum / (count - 2);

            adcValue[ i ].average     = average;

            adcValue[ i ].sum 		   = 0;
            adcValue[ i ].sum_count 	= 0;
            adcValue[ i ].min 	      = adcConfList[ i ].min;
            adcValue[ i ].max 	      = adcConfList[ i ].max;
        }
    }
}



