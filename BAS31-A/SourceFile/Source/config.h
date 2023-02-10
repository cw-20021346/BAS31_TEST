#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define	VERSION   2		// checksum : 9701
//#define	VERSION   3			// �������� ���� ���� by. ������1�� // checksum : 7F97
#define	VERSION   4			// �ڵ� ���˻� �߰�  by. ���ȯ ����  /// checksum: 116A

//Change_config


/************************************************************/
/* ��� config SET ���� ��� SET �ϼ���.*/
/************************************************************/
/* SET: Final Step, CLEAR: Next Step */
#define CONFIG_MOTOR_STEP          1

#define CONFIG_UART_REGISTER        1

/************************************************************/
/* ��� config CLEAR ���� ��� CLEAR �ϼ���.*/
/************************************************************/


/* SET: 70%, CLEAR: 100% */
#define CONFIG_PRE_HEATING_NOISE_REDUCE   0

#define CONFIG_TEST                 0

/* SET: PWM Control, CLEAR: FULL Control */
#define CONFIG_VALVE_PWM            0

/* SET: ���� �������, CLEAR: normal Mode */
#define CONFIG_INFINITY_MODE        0

/* SET: Remote Model, CLEAR: Key Model */
#define CONFIG_REMOTE               0

/* SET: Test mode (10msec counter), CLEAR: Normal mode (1min counter) */
#define CONFIG_AUTO_DRAIN_TEST      0

/* SET: ��տϷ�� LED ON , CLEAR: ��� �Ϸ�� LED OFF */
#define CONFIG_STER_COMPLETE_DISPLAY               0


#endif /* __CONFIG_H__ */
