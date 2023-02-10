#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define	VERSION   2		// checksum : 9701
//#define	VERSION   3			// 워터펌프 누수 개선 by. 김진우1급 // checksum : 7F97
#define	VERSION   4			// 자동 물검사 추가  by. 김민환 선임  /// checksum: 116A


/************************************************************/
/* 양산 config SET 양산시 모두 SET 하세요.*/
/************************************************************/
/* SET: Final Step, CLEAR: Next Step */
#define CONFIG_MOTOR_STEP          1

#define CONFIG_UART_REGISTER        1

/************************************************************/
/* 양산 config CLEAR 양산시 모두 CLEAR 하세요.*/
/************************************************************/


/* SET: 70%, CLEAR: 100% */
#define CONFIG_PRE_HEATING_NOISE_REDUCE   0

#define CONFIG_TEST                 0

/* SET: PWM Control, CLEAR: FULL Control */
#define CONFIG_VALVE_PWM            0

/* SET: 무한 세정모드, CLEAR: normal Mode */
#define CONFIG_INFINITY_MODE        0

/* SET: Remote Model, CLEAR: Key Model */
#define CONFIG_REMOTE               0

/* SET: Test mode (10msec counter), CLEAR: Normal mode (1min counter) */
#define CONFIG_AUTO_DRAIN_TEST      0

/* SET: 살균완료시 LED ON , CLEAR: 살균 완료시 LED OFF */
#define CONFIG_STER_COMPLETE_DISPLAY               0


#endif /* __CONFIG_H__ */
