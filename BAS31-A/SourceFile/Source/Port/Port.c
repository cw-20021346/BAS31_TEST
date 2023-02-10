#include "hw.h"

#include "Port.h"

#include "Power.h"
#include "PowerSave.h"

/* while문 */
void Port_Control (void)
{
    /* port control */
    if( GetPowerOnOff() == TRUE )
    {
        TURN_ON_3_3V();

        if( GetPowerSaveLED() == FALSE)
        {
            TURN_ON_FLOW_CONTROL(); //유량 측정 
            TURN_ON_ZERO_CONTROL();  //제로크로싱 출력          
        }
        else
        {
            TURN_OFF_FLOW_CONTROL(); //유량 측정 
            TURN_OFF_ZERO_CONTROL();  //제로크로싱 출력    
        }
    }
    else
    {

        TURN_OFF_3_3V();
        
        TURN_OFF_FLOW_CONTROL(); //유량 측정 
        TURN_OFF_ZERO_CONTROL();  //제로크로싱 출력 

        TURN_OFF_SEAT_HEATER();
    	TURN_OFF_WATER_HEATER();
        TURN_OFF_DRY_HEATER();
        TURN_OFF_DRY_RELAY();
        TURN_OFF_FAN();
        
        TURN_OFF_AIR_PUMP();
        TURN_OFF_STER();
    }
}

void PORTCLEAR()
{
    INIT_LED_SCAN();
    INIT_LED_SELECT();
    
    TURN_OFF_WIDE_STEP_0();
    TURN_OFF_WIDE_STEP_1();
    TURN_OFF_WIDE_STEP_2();
    TURN_OFF_WIDE_STEP_3();
  
    TURN_OFF_PRESS_STEP_0();
    TURN_OFF_PRESS_STEP_1();
    TURN_OFF_PRESS_STEP_2();
    TURN_OFF_PRESS_STEP_3();
  
    TURN_OFF_CLEAN_STEP_0();
    TURN_OFF_CLEAN_STEP_1();
    TURN_OFF_CLEAN_STEP_2();
    TURN_OFF_CLEAN_STEP_3();
  
    TURN_OFF_BIDET_STEP_0();
    TURN_OFF_BIDET_STEP_1();
    TURN_OFF_BIDET_STEP_2();
    TURN_OFF_BIDET_STEP_3();

    TURN_OFF_BUZZER();

    TURN_OFF_DRY_RELAY();
    TURN_OFF_DRY_HEATER();
    TURN_OFF_FAN();

    TURN_OFF_SEAT_HEATER();

    TURN_OFF_WATER_HEATER();
#if 0
    TURN_OFF_WATER_HEATER_1();
    TURN_OFF_WATER_HEATER_2();
#endif
    TURN_OFF_FLOW_CONTROL();
    TURN_OFF_ZERO_CONTROL();
 
    TURN_OFF_STER();

    TURN_OFF_AIR_PUMP();

}


