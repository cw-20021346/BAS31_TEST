#ifndef __FACTORYMODE_H__
#define __FACTORYMODE_H__

#include "prj_type.h"


typedef enum
{
    FACTORY_LED_0,
    FACTORY_LED_1,
    FACTORY_LED_2,
    FACTORY_LED_3,
} FactoryLEDId_T;

typedef enum
{
    AUTO_TEST_INIT,
    AUTO_TEST_CLEAN_INIT,    
    AUTO_TEST_CLEAN_START,
    AUTO_TEST_CLEAN_PRESS,
    AUTO_TEST_CLEAN_WIDE,
    AUTO_TEST_CLEAN_NOZZLE,
    AUTO_TEST_CLEAN_DELAY,
    AUTO_TEST_CLEAN_END,

    AUTO_TEST_BIDET_INIT,
    AUTO_TEST_BIDET_START,
    AUTO_TEST_BIDET_PRESS,
    AUTO_TEST_BIDET_WIDE,
    AUTO_TEST_BIDET_NOZZLE,
    AUTO_TEST_BIDET_DELAY,
    AUTO_TEST_BIDET_END,

    AUTO_TEST_DRY_START,
    AUTO_TEST_DRY_END,
    
}AutoTestStepId_T;

void InitFactory ( void );

void SetFactoryStart ( U8 mu8Val);
U8 GetFactoryStart ( void );
void SetFactoryReady ( U8 mu8Val);
U8 GetFactoryReady ( void );
    

void SetFactoryLED ( U8 mu8Val);
U8 GetFactoryLED ( void );
void SetFactoryTest ( U8 mu8Val);
U8 GetFactoryTest ( void );

void SetFactoryTest_Remote ( U8 mu8Val);
U8 GetFactoryTest_Remote ( void );
void SetFactoryAutoTest ( U8 mu8Val);
U8 GetFactoryAutoTest ( void );
void SetFactoryAutoTestStep ( U8 mu8Val);
U8 GetFactoryAutoTestStep ( void );

void Evt_100msec_Factory_Handler(void);
void Evt_100msec_Test_Handler(void);
/*****************************************************************************************
 FUNCTION NAME : void Evt_100msec_AutoTest_Handler(void)

 DESCRIPTION
 *  -. 100msec scheduler
 *  -. 자동 테스트 Sequence
 
 PARAMETERS: void

 RETURN VALUE:  void

 NOTES:
 
*****************************************************************************************/
void Evt_100msec_AutoTest_Handler(void);


void FactoryKeyCheck ( U32 mu32KeyVal );


#endif
