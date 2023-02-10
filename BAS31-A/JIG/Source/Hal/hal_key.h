#ifndef __HAL_KEY_H__
#define __HAL_KEY_H__

//#define MATRIX_SCAN_MAX     4     /* SCAN 0~3 4ea */

/* 키 입력 상태 값 */
#define  HAL_KEY_NONE                   0x00000000

#define  HAL_KEY_1                      0x00000001      /* nozzle clean */
#define  HAL_KEY_2                      0x00000002      /* Power */
#define  HAL_KEY_3                      0x00000004      /* Water Temp */
#define  HAL_KEY_4                      0x00000008      /* Seat Temp */

#define  HAL_KEY_5                      0x00000010      /* Ster */
#define  HAL_KEY_6                      0x00000020      /* Dry */
#define  HAL_KEY_7                      0x00000040      /* Nozzle Back */
#define  HAL_KEY_8                      0x00000080      /* Press Down */

#define  HAL_KEY_9                      0x00000100      /* Nozzle Front */
#define  HAL_KEY_10                     0x00000200      /* Press Up */
#define  HAL_KEY_11                     0x00000400      /* Mode(State) */
#define  HAL_KEY_12                     0x00000800      /* Bidet */

#define  HAL_KEY_13                     0x00001000      /* Clean */
#define  HAL_KEY_14                     0x00002000      /* Stop */
#define  HAL_KEY_15                     0x00004000
#define  HAL_KEY_16                     0x00008000

#define  HAL_KEY_SIDE_1                 0x00010000      /* Side1 */
#define  HAL_KEY_SIDE_2                 0x00020000      /* Side2 */
#define  HAL_KEY_SIDE_3                 0x00040000      /* Side3 */
#define  HAL_KEY_SIDE_4                 0x00080000      /* Side4 */



U32 HAL_GetKeyVal(void);
void HAL_GetScanKey( void );


#if 0
#define  HAL_LEVER_NONE       0x00        
#define  HAL_LEVER_ICE        0x01
#define  HAL_LEVER_WATER      0x02
U8 HAL_GetLeverVal(void);
/* 얼음 추출 레버 상태 값 */
U8 HAL_GetLeverIce(void);

/* 물 추출 레버 상태 값 */
U8 HAL_GetLeverWater(void);
#endif


#endif /* __HAL_KEY_H__ */
