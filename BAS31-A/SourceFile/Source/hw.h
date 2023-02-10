#ifndef __HW_H__
#define __HW_H__

#include "mcu.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_wdt.h"
#include "r_cg_timer.h"
#include "r_cg_adc.h"
#include "r_cg_serial.h"
#include "r_cg_macrodriver.h"

//Change_hw



/* Code Generate*/
//1. Base Timer     INTTM00    __interrupt static void BaseTimerInterrupt(void)    Prosecc_sys_event.c
//2. 200usec Timer  INTTM01    __interrupt static void INT_200us(void)             Prosecc_sys_event.c
//3. Valve Timer    INTTM07     Valve.c

//4. ADC            INTAD
//5. Port interrupt INTP4       __interrupt static void INT_ZeroCross(void)        Main.c
//6. INT_Remote     INTP11      __interrupt static void INT_Remote_IR(void)        Remote.c

//#define INT_Flow()            static void r_intc6_interrupt(void)



#define  P_TEST                     P15.6

/******************************************************************************/
/* LED & KEY */
/******************************************************************************/
/* Matrix */
#define SCAN_5              P5.4
#define SCAN_4              P5.3
#define SCAN_3              P5.2
#define SCAN_2              P5.1
#define SCAN_1              P5.0

#define	INIT_LED_SCAN()	\
	do{	\
		SCAN_1 = 1;	\
		SCAN_2 = 1;	\
		SCAN_3 = 1;	\
		SCAN_4 = 1;	\
	}while(0)
	
#define SELECT_4            P1.7
#define SELECT_3            P5.7
#define SELECT_2            P5.6
#define SELECT_1            P5.5

#define	INIT_LED_SELECT()	\
	do{	\
		SELECT_1 = 0;	\
		SELECT_2 = 0;	\
		SELECT_3 = 0;	\
		SELECT_4 = 0;	\
	}while(0)


#define  KEY_4                       P1.3
#define  GET_STATUS_KEY_4()          ( KEY_4 )

#define  KEY_3                       P1.4
#define  GET_STATUS_KEY_3()          ( KEY_3 )

#define  KEY_2                       P1.5
#define  GET_STATUS_KEY_2()          ( KEY_2 )

#define  KEY_1                       P1.6
#define  GET_STATUS_KEY_1()          ( KEY_1 )


#define  P_LED_MOOD_BLUE		            P6.2
#define  TURN_ON_LED_MOOD_BLUE()            do{ P_LED_MOOD_BLUE = 0; }while(0)
#define  TURN_OFF_LED_MOOD_BLUE()           do{ P_LED_MOOD_BLUE = 1; }while(0)
#define  GET_STATUS_LED_MOOD_BLUE()         ( P_LED_MOOD_BLUE )

#define  P_LED_MOOD_RED		                P6.3
#define  TURN_ON_LED_MOOD_RED()             do{ P_LED_MOOD_RED = 0; }while(0)
#define  TURN_OFF_LED_MOOD_RED()            do{ P_LED_MOOD_RED = 1; }while(0)
#define  GET_STATUS_LED_MOOD_RED()          ( P_LED_MOOD_RED )

/******************************************************************************/
/* Motor */
/******************************************************************************/
/*수압모터*/
#define  P_PRESS_STEP_0                     P11.1
#define  TURN_ON_PRESS_STEP_0()             do{ P_PRESS_STEP_0 = 1; }while(0)
#define  TURN_OFF_PRESS_STEP_0()            do{ P_PRESS_STEP_0 = 0; }while(0)
#define  GET_STATUS_PRESS_STEP_0()          ( P_PRESS_STEP_0 )

#define  P_PRESS_STEP_1                     P14.6
#define  TURN_ON_PRESS_STEP_1()             do{ P_PRESS_STEP_1 = 1; }while(0)
#define  TURN_OFF_PRESS_STEP_1()            do{ P_PRESS_STEP_1 = 0; }while(0)
#define  GET_STATUS_PRESS_STEP_1()          ( P_PRESS_STEP_1 )

#define  P_PRESS_STEP_2                     P14.7
#define  TURN_ON_PRESS_STEP_2()             do{ P_PRESS_STEP_2 = 1; }while(0)
#define  TURN_OFF_PRESS_STEP_2()            do{ P_PRESS_STEP_2 = 0; }while(0)
#define  GET_STATUS_PRESS_STEP_2()          ( P_PRESS_STEP_2 )

#define  P_PRESS_STEP_3                     P10.0
#define  TURN_ON_PRESS_STEP_3()             do{ P_PRESS_STEP_3 = 1; }while(0)
#define  TURN_OFF_PRESS_STEP_3()            do{ P_PRESS_STEP_3 = 0; }while(0)
#define  GET_STATUS_PRESS_STEP_3()          ( P_PRESS_STEP_3 )

/*Wide 모터*/
#define  P_WIDE_STEP_0                     P14.4
#define  TURN_ON_WIDE_STEP_0()             do{ P_WIDE_STEP_0 = 1; }while(0)
#define  TURN_OFF_WIDE_STEP_0()            do{ P_WIDE_STEP_0 = 0; }while(0)
#define  GET_STATUS_WIDE_STEP_0()          ( P_WIDE_STEP_0 )

#define  P_WIDE_STEP_1                     P14.3
#define  TURN_ON_WIDE_STEP_1()             do{ P_WIDE_STEP_1 = 1; }while(0)
#define  TURN_OFF_WIDE_STEP_1()            do{ P_WIDE_STEP_1 = 0; }while(0)
#define  GET_STATUS_WIDE_STEP_1()          ( P_WIDE_STEP_1 )

#define  P_WIDE_STEP_2                     P14.2
#define  TURN_ON_WIDE_STEP_2()             do{ P_WIDE_STEP_2 = 1; }while(0)
#define  TURN_OFF_WIDE_STEP_2()            do{ P_WIDE_STEP_2 = 0; }while(0)
#define  GET_STATUS_WIDE_STEP_2()          ( P_WIDE_STEP_2 )

#define  P_WIDE_STEP_3                     P14.1
#define  TURN_ON_WIDE_STEP_3()             do{ P_WIDE_STEP_3 = 1; }while(0)
#define  TURN_OFF_WIDE_STEP_3()            do{ P_WIDE_STEP_3 = 0; }while(0)
#define  GET_STATUS_WIDE_STEP_3()          ( P_WIDE_STEP_3 )

/*Clean 노즐*/
#define  P_CLEAN_STEP_0                     P3.0
#define  TURN_ON_CLEAN_STEP_0()             do{ P_CLEAN_STEP_0 = 1; }while(0)
#define  TURN_OFF_CLEAN_STEP_0()            do{ P_CLEAN_STEP_0 = 0; }while(0)
#define  GET_STATUS_CLEAN_STEP_0()          ( P_CLEAN_STEP_0 )

#define  P_CLEAN_STEP_1                     P8.7
#define  TURN_ON_CLEAN_STEP_1()             do{ P_CLEAN_STEP_1 = 1; }while(0)
#define  TURN_OFF_CLEAN_STEP_1()            do{ P_CLEAN_STEP_1 = 0; }while(0)
#define  GET_STATUS_CLEAN_STEP_1()          ( P_CLEAN_STEP_1 )

#define  P_CLEAN_STEP_2                     P8.6
#define  TURN_ON_CLEAN_STEP_2()             do{ P_CLEAN_STEP_2 = 1; }while(0)
#define  TURN_OFF_CLEAN_STEP_2()            do{ P_CLEAN_STEP_2 = 0; }while(0)
#define  GET_STATUS_CLEAN_STEP_2()          ( P_CLEAN_STEP_2 )

#define  P_CLEAN_STEP_3                     P8.5
#define  TURN_ON_CLEAN_STEP_3()             do{ P_CLEAN_STEP_3 = 1; }while(0)
#define  TURN_OFF_CLEAN_STEP_3()            do{ P_CLEAN_STEP_3 = 0; }while(0)
#define  GET_STATUS_CLEAN_STEP_3()          ( P_CLEAN_STEP_3 )

/*Bidet 모터*/
#define  P_BIDET_STEP_0                     P8.3
#define  TURN_ON_BIDET_STEP_0()             do{ P_BIDET_STEP_0 = 1; }while(0)
#define  TURN_OFF_BIDET_STEP_0()            do{ P_BIDET_STEP_0 = 0; }while(0)
#define  GET_STATUS_BIDET_STEP_0()          ( P_BIDET_STEP_0 )

#define  P_BIDET_STEP_1                     P8.2
#define  TURN_ON_BIDET_STEP_1()             do{ P_BIDET_STEP_1 = 1; }while(0)
#define  TURN_OFF_BIDET_STEP_1()            do{ P_BIDET_STEP_1 = 0; }while(0)
#define  GET_STATUS_BIDET_STEP_1()          ( P_BIDET_STEP_1 )

#define  P_BIDET_STEP_2                     P8.1
#define  TURN_ON_BIDET_STEP_2()             do{ P_BIDET_STEP_2 = 1; }while(0)
#define  TURN_OFF_BIDET_STEP_2()            do{ P_BIDET_STEP_2 = 0; }while(0)
#define  GET_STATUS_BIDET_STEP_2()          ( P_BIDET_STEP_2 )

#define  P_BIDET_STEP_3                     P8.0
#define  TURN_ON_BIDET_STEP_3()             do{ P_BIDET_STEP_3 = 1; }while(0)
#define  TURN_OFF_BIDET_STEP_3()            do{ P_BIDET_STEP_3 = 0; }while(0)
#define  GET_STATUS_BIDET_STEP_3()          ( P_BIDET_STEP_3 )

/******************************************************************************/
/* Dry */
/******************************************************************************/
#define  P_FAN_ONOFF		                P4.3 
#define  TURN_ON_FAN()                      do{ P_FAN_ONOFF = 1; }while(0)
#define  TURN_OFF_FAN()                     do{ P_FAN_ONOFF = 0; }while(0)
#define  GET_STATUS_FAN()                   ( P_FAN_ONOFF )
     
#define  P_DRY_RELAY_ONOFF		            P6.4
#define  TURN_ON_DRY_RELAY()                do{ P_DRY_RELAY_ONOFF = 1; }while(0)
#define  TURN_OFF_DRY_RELAY()               do{ P_DRY_RELAY_ONOFF = 0; }while(0)
#define  GET_STATUS_DRY_RELAY()             ( P_DRY_RELAY_ONOFF )
       
#define  P_DRY_HEATER_ONOFF		            P6.6
#define  TURN_ON_DRY_HEATER()               do{ P_DRY_HEATER_ONOFF = 1; }while(0)
#define  TURN_OFF_DRY_HEATER()              do{ P_DRY_HEATER_ONOFF = 0; }while(0)
#define  GET_STATUS_DRY_HEATER()            ( P_DRY_HEATER_ONOFF )

/******************************************************************************/
/* Seat */
/******************************************************************************/     
#define  P_SEAT     		                P1.0
#define  GET_STATUS_SEAT()                 ( P_SEAT )

#define  P_SEAT_HEATER_ONOFF		         P6.5
#define  TURN_ON_SEAT_HEATER()               do{ P_SEAT_HEATER_ONOFF = 1; }while(0)
#define  TURN_OFF_SEAT_HEATER()              do{ P_SEAT_HEATER_ONOFF = 0; }while(0)
#define  GET_STATUS_SEAT_HEATER()            ( P_SEAT_HEATER_ONOFF )

/******************************************************************************/
/* Hot */
/******************************************************************************/ 
#define  P_WATER_HEATER_ONOFF		       P6.7
#define  TURN_ON_WATER_HEATER()            do{ P_WATER_HEATER_ONOFF = 1; }while(0)
#define  TURN_OFF_WATER_HEATER()           do{ P_WATER_HEATER_ONOFF = 0; }while(0)
#define  GET_STATUS_WATER_HEATER()         ( P_WATER_HEATER_ONOFF )

/******************************************************************************/
/* Sterilize */
/******************************************************************************/
#define  P_STER_ONOFF		            P11.0
#define  TURN_ON_STER()                 if( GET_STATUS_FAN() == FALSE ) do{ P_STER_ONOFF = 1; }while(0)
#define  TURN_OFF_STER()                do{ P_STER_ONOFF = 0; }while(0)
#define  GET_STATUS_STER()              ( P_STER_ONOFF )

#define  P_WATER_PUMP_ONOFF             P10.1
#define  TURN_ON_WATER_PUMP()           do{ P_WATER_PUMP_ONOFF = 1; }while(0)
#define  TURN_OFF_WATER_PUMP()          do{ P_WATER_PUMP_ONOFF = 0; }while(0)
#define  GET_STATUS_WATER_PUMP()        ( P_WATER_PUMP_ONOFF )

/******************************************************************************/
/* Function */
/******************************************************************************/
#define  P_WATER_LEVEL                      P10.2
#define  GET_STATUS_WATER_LEVEL()           ( P_WATER_LEVEL )

/* 물검사를 위해 TURN ON시 상시 수위 감지로 인식 */
#define  P_WATER_LEVEL_TEST                 P13.0
#define  TURN_ON_WATER_LEVEL_TEST()         do{ P_WATER_LEVEL_TEST = 1; }while(0)
#define  TURN_OFF_WATER_LEVEL_TEST()        do{ P_WATER_LEVEL_TEST = 0; }while(0)
#define  GET_STATUS_WATER_LEVEL_TEST()      ( P_WATER_LEVEL_TEST )

#define  P_AIR_PUMP_ONOFF               P7.4
#define  TURN_ON_AIR_PUMP()             do{ P_AIR_PUMP_ONOFF = 1; }while(0)
#define  TURN_OFF_AIR_PUMP()            do{ P_AIR_PUMP_ONOFF = 0; }while(0)
#define  GET_STATUS_AIR_PUMP()          ( P_AIR_PUMP_ONOFF )

//#define P_Buzzer_PWM                    P4.2
#define  P_BUZZER_ONOFF		            P4.1
#define  TURN_ON_BUZZER()               do{ P_BUZZER_ONOFF = 1; }while(0)
#define  TURN_OFF_BUZZER()              do{ P_BUZZER_ONOFF = 0; }while(0)
#define  GET_STATUS_BUZZER()            ( P_BUZZER_ONOFF )

#define P_VALVE_PWM                   P14.5
#define  TURN_ON_VALVE()               do{ P_VALVE_PWM = 1; }while(0)
#define  TURN_OFF_VALVE()              do{ P_VALVE_PWM = 0; }while(0)
#define  GET_STATUS_VALVE()            ( P_VALVE_PWM )

#define  P_WATER_AUTO_FLUSHING              P4.4
#define  TURN_ON_WATER_AUTO_FLUSHING()      do{ P_WATER_AUTO_FLUSHING = 1; }while(0)
#define  TURN_OFF_WATER_AUTO_FLUSHING()     do{ P_WATER_AUTO_FLUSHING = 0; }while(0)
#define  GET_STATUS_WATER_AUTO_FLUSHING()   ( P_WATER_AUTO_FLUSHING )


/******************************************************************************/
/* UART & I2C */
/******************************************************************************/
//#define P_I2C_SCLA0                     P6.0
//#define P_I2C_SDAA0                     P6.1



/******************************************************************************/
/* ADC */
/******************************************************************************/
#if 0
#define P_SEAT_AD           ADC0
#define P_IN_AD             ADC1
#define P_OUT_AD            ADC2
#define P_EW_AD             ADC3
#define P_VALVE_AD          ADC4
#define P_VOLTAGE_AD        ADC5
#endif

/******************************************************************************/
/* Remote */
/******************************************************************************/
#define  P_REMOTE_IR                     P7.7
#define  GET_STATUS_REMOTE_IR()         ( P_REMOTE_IR )

#define  P_REMOTE_POWER		            P7.6
#define  TURN_ON_REMOTE_POWER()         do{ P_REMOTE_POWER = 0; }while(0)
#define  TURN_OFF_REMOTE_POWER()        do{ P_REMOTE_POWER = 1; }while(0)
#define  GET_STATUS_REMOTE_POWER()      ( P_REMOTE_POWER )

#define  P_3_3V                         P7.5
#define  TURN_ON_3_3V()                 do{ P_3_3V = 1; }while(0)
#define  TURN_OFF_3_3V()                do{ P_3_3V = 0; }while(0)
#define  GET_STATUS_3_3V()              ( P_3_3V )

/******************************************************************************/
/* Wi-fi */
/******************************************************************************/
//#define P_WIFI_RXD                      P0.3
//#define P_WIFI_TXD                      P0.2

#define  P_WIFI_POWER		            P0.4
#define  TURN_ON_WIFI_POWER()           do{ P_WIFI_POWER = 1; }while(0)
#define  TURN_OFF_WIFI_POWER()          do{ P_WIFI_POWER = 0; }while(0)
#define  GET_STATUS_WIFI_POWER()        ( P_WIFI_POWER )

#define  P_WIFI_RSTN		            P0.1
#define  TURN_ON_WIFI_RSTN()           do{ P_WIFI_RSTN = 1; }while(0)
#define  TURN_OFF_WIFI_RSTN()          do{ P_WIFI_RSTN = 0; }while(0)
#define  GET_STATUS_WIFI_RSTN()        ( P_WIFI_RSTN )

/******************************************************************************/
/* Side LED * KEY */
/******************************************************************************/
#define  P_SIDE_KEY_1		             P7.3
#define  GET_STATUS_SIDE_KEY_1()         ( P_SIDE_KEY_1 )

#define  P_SIDE_KEY_2		             P7.2
#define  GET_STATUS_SIDE_KEY_2()         ( P_SIDE_KEY_2 )

#define  P_SIDE_KEY_3		             P7.1
#define  GET_STATUS_SIDE_KEY_3()         ( P_SIDE_KEY_3 )

#define  P_SIDE_LED		                 P7.0
#define  TURN_ON_SIDE_LED()              do{ P_SIDE_LED = 1; }while(0)
#define  TURN_OFF_SIDE_LED()             do{ P_SIDE_LED = 0; }while(0)
#define  GET_STATUS_SIDE_LED()           ( P_SIDE_LED )

/******************************************************************************/
/* Not Used */
/******************************************************************************/
#define P_FLOW_IN                          P14.0   /*port interrupt INTP6 */
#define  P_FLOW_CONTROL                    P0.0
#define  TURN_ON_FLOW_CONTROL()            do{ P_FLOW_CONTROL = 1; }while(0)
#define  TURN_OFF_FLOW_CONTROL()           do{ P_FLOW_CONTROL = 0; }while(0)
#define  GET_STATUS_FLOW_CONTROL()         ( P_FLOW_CONTROL )

#define P_ZERO_CROSS                       P3.1  /*port interrupt INTP4 */
#define GET_STATUS_ZERO_CROSS()            ( P_ZERO_CROSS )

#define  P_ZERO_CONTROL                    P0.6
#define  TURN_ON_ZERO_CONTROL()            do{ P_ZERO_CONTROL = 1; }while(0)
#define  TURN_OFF_ZERO_CONTROL()           do{ P_ZERO_CONTROL = 0; }while(0)
#define  GET_STATUS_ZERO_CONTROL()         ( P_ZERO_CONTROL )

#define  P_LOW_PRESS_PUMP_ONOFF         P0.5
#define  TURN_ON_LOW_PRESS_PUMP()       do{ P_LOW_PRESS_PUMP_ONOFF = 1; }while(0)
#define  TURN_OFF_LOW_PRESS_PUMP()      do{ P_LOW_PRESS_PUMP_ONOFF = 0; }while(0)
#define  GET_STATUS_LOW_PRESS_PUMP()    ( P_LOW_PRESS_PUMP_ONOFF )

#define  P_DRY_STEP_0                   P4.5
#define  TURN_ON_DRY_STEP_0()           do{ P_DRY_STEP_0 = 1; }while(0)
#define  TURN_OFF_DRY_STEP_0()          do{ P_DRY_STEP_0 = 0; }while(0)
#define  GET_STATUS_DRY_STEP_0()        ( P_DRY_STEP_0 )

#define  P_DRY_STEP_1                   P4.6
#define  TURN_ON_DRY_STEP_1()           do{ P_DRY_STEP_1 = 1; }while(0)
#define  TURN_OFF_DRY_STEP_1()          do{ P_DRY_STEP_1 = 0; }while(0)
#define  GET_STATUS_DRY_STEP_1()        ( P_DRY_STEP_1 )

#define  P_DRY_STEP_2                   P4.7
#define  TURN_ON_DRY_STEP_2()           do{ P_DRY_STEP_2 = 1; }while(0)
#define  TURN_OFF_DRY_STEP_2()          do{ P_DRY_STEP_2 = 0; }while(0)
#define  GET_STATUS_DRY_STEP_2()        ( P_DRY_STEP_2 )

#define  P_DRY_STEP_3                   P12.0
#define  TURN_ON_DRY_STEP_3()           do{ P_DRY_STEP_3 = 1; }while(0)
#define  TURN_OFF_DRY_STEP_3()          do{ P_DRY_STEP_3 = 0; }while(0)
#define  GET_STATUS_DRY_STEP_3()        ( P_DRY_STEP_3 )



#if 0
#define  P_NOT_USED                     P8.4
#define  P_NOT_USED                     P15.6
#define  P_NOT_USED                     P15.5
#define  P_NOT_USED                     P15.4
#define  P_NOT_USED                     P15.3
#define  P_NOT_USED                     P15.2
#define  P_NOT_USED                     P15.1
#define  P_NOT_USED                     P15.0
#define  P_NOT_USED                     P2.7
#define  P_NOT_USED                     P2.6

/* input Only*/
#define  P_NOT_USED                     P13.7
#define  P_NOT_USED                     P12.1
#define  P_NOT_USED                     P12.2
#define  P_NOT_USED                     P12.3
#define  P_NOT_USED                     P12.4
#endif

#endif /* __HW_H__ */
