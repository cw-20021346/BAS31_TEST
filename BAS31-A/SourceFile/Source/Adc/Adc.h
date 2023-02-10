#ifndef __ADC_H__
#define __ADC_H__

#include "prj_type.h"

//온도과상승에러
#define WATER_ERROR       200.0f //
#define WATER_OVER_CHECK   46.0f
#define WATER_OVER_RELEASE 38.0f
#define WATER_LOW_TEMP    0.0f   //200.0
#define WATER_HIGH_TEMP   50.0f  //250.0 

//변좌과상승에러
#define SEAT_ERROR        200.0f  //물과 항상 같은 데이터 유지 필요
#define SEAT_LOW_TEMP     0.0f    //물과 항상 같은 데이터 유지 필요
#define SEAT_HIGH_TEMP    51.0f   //물과 항상 같은 데이터 유지 필요

void InitADC(void);
void Evt_10msec_ADC_Handler(void);

U16 GetADCDelay(void);
U8 GetInTemperatureSection ( void );

/*ADC 최종값 Get함수*/
F32 GetSeatSensor(void);
F32 GetInSensor(void);
F32 GetOutSensor(void);
U16 GetEWSensor(void);
U16 GetVoltageSensor(void);
U16 GetValveSensor(void);

//온도센서
F32 GetWaterTemperature(U16 mu16ADTemperature);
F32 GetSeatTemperature(U16 mu16ADSeatTemperature);
#endif
