#ifndef __ADC_H__
#define __ADC_H__

#include "prj_type.h"

//�µ�����¿���
#define WATER_ERROR       200.0f //
#define WATER_OVER_CHECK   46.0f
#define WATER_OVER_RELEASE 38.0f
#define WATER_LOW_TEMP    0.0f   //200.0
#define WATER_HIGH_TEMP   50.0f  //250.0 

//���°���¿���
#define SEAT_ERROR        200.0f  //���� �׻� ���� ������ ���� �ʿ�
#define SEAT_LOW_TEMP     0.0f    //���� �׻� ���� ������ ���� �ʿ�
#define SEAT_HIGH_TEMP    51.0f   //���� �׻� ���� ������ ���� �ʿ�

void InitADC(void);
void Evt_10msec_ADC_Handler(void);

U16 GetADCDelay(void);
U8 GetInTemperatureSection ( void );

/*ADC ������ Get�Լ�*/
F32 GetSeatSensor(void);
F32 GetInSensor(void);
F32 GetOutSensor(void);
U16 GetEWSensor(void);
U16 GetVoltageSensor(void);
U16 GetValveSensor(void);

//�µ�����
F32 GetWaterTemperature(U16 mu16ADTemperature);
F32 GetSeatTemperature(U16 mu16ADSeatTemperature);
#endif
