#include "Adc.h"
#include "hal_adc.h"

#define ADC_DELAY           100     /*10msec * 100 = 1sec */



//========= 입수 및 출수 온도 Sensor Table ==================
//계산법: SAR
//SAR: Vin/(Vref+Vin)*1024+0.5의 정수값
//PB3-43-S19

//0~4.9도
 const U16 gu16THERMISTOR_Table_0[]={
  //0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,
  767, 767, 766, 765, 764, 763, 762, 761, 761, 760,
//1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9,
  759, 758, 757, 756, 755, 754, 754, 753, 752, 751,
//2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9,
  750, 749, 748, 747, 747, 746, 745, 744, 743, 742,
//3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9,
  741, 740, 740, 739, 738, 737, 736, 735, 734, 733,
//4.0, 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9,
  732, 731, 731, 730, 729, 728, 727, 726, 725, 724
};
//5~9.9도
 const U16 gu16THERMISTOR_Table_5[]={
//5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9,  
  723, 722, 721, 721, 720, 719, 718, 717, 716, 715,
//6.0, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9,  
  714, 713, 712, 711, 710, 710, 709, 708, 707, 706,
//7.0, 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 7.8, 7.9,   
  705, 704, 703, 702, 701, 700, 699, 698, 697, 696,
//8.0, 8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 8.8, 8.9, 
  696, 695, 694, 693, 692, 691, 690, 689, 688, 687,
//9.0, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.9,
  686, 685, 684, 683, 682, 681, 680, 679, 679, 678
};
//10~14.9도
 const U16 gu16THERMISTOR_Table_10[]={
//10.0, 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9,
   677,  676,  675,  674,  673,  672,  671,  670,  669,  668,
//11.0, 11.1, 11.2, 11.3, 11.4, 11.5, 11.6, 11.7, 11.8, 11.9,
   667,  666,  665,  664,  663,  662,  661,  660,  659,  658,
//12.0, 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9,   
   657,  656,  656,  655,  654,  653,  652,  651,  650,  649,
//13.0, 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.7, 13.8, 13.9,   
   648,  647,  646,  645,  644,  643,  642,  641,  640,  639,
//14.0, 14.1, 14.2, 14.3, 14.4, 14.5, 14.6, 14.7, 14.8, 14.9,   
   638,  637,  636,  635,  634,  633,  632,  631,  630,  629
};
//15~19.9도
 const U16 gu16THERMISTOR_Table_15[]={
//15.0, 15.1, 15.2, 15.3, 15.4, 15.5, 15.6, 15.7, 15.8, 15.9,    
   628,  627,  626,  625,  624,  623,  622,  621,  620,  619,
//16.0, 16.1, 16.2, 16.3, 16.4, 16.5, 16.6, 16.7, 16.8, 16.9,
   618,  617,  617,  616,  615,  614,  613,  612,  611,  610,
//17.0, 17.1, 17.2, 17.3, 17.4, 17.5, 17.6, 17.7, 17.8, 17.9,
   609,  608,  607,  606,  605,  604,  603,  602,  601,  600,
//18.0, 18.1, 18.2, 18.3, 18.4, 18.5, 18.6, 18.7, 18.8, 18.9, 
   599,  598,  597,  596,  595,  594,  593,  592,  591,  590,
//19.0, 19.1, 19.2, 19.3, 19.4, 19.5, 19.6, 19.7, 19.8, 19.9,
   589,  588,  587,  586,  585,  584,  583,  582,  581, 580
};
//20~24.9도
 const U16 gu16THERMISTOR_Table_20[]={  
//20.0, 20.1, 20.2, 20.3, 20.4, 20.5, 20.6, 20.7, 20.8, 20.9,
   579,  578,  577,  576,  575,  574,  573,  572,  571,  570,
//21.0, 21.1, 21.2, 21.3, 21.4, 21.5, 21.6, 21.7, 21.8, 21.9,
   569,  568,  567,  566,  565,  564,  563,  562,  561,  561,
//22.0, 22.1, 22.2, 22.3, 22.4, 22.5, 22.6, 22.7, 22.8, 22.9,
   560,  559,  558,  557,  556,  555,  554,  553,  552,  551,
//23.0, 23.1, 23.2, 23.3, 23.4, 23.5, 23.6, 23.7, 23.8, 23.9,  
   550,  549,  548,  547,  546,  545,  544,  543,  542,  541,
//24.0, 24.1, 24.2, 24.3, 24.4, 24.5, 24.6, 24.7, 24.8, 24.9,
   540,  539,  538,  537,  536,  535,  534,  533,  532,  531
};
//25~29.9도
 const U16 gu16THERMISTOR_Table_25[]={ 
//25.0, 25.1, 25.2, 25.3, 25.4, 25.5, 25.6, 25.7, 25.8, 25.9,
   530,  529,  528,  527,  526,  525,  524,  523,  523,  522,
//26.0, 26.1, 26.2, 26.3, 26.4, 26.5, 26.6, 26.7, 26.8, 26.9,  
   521,  520,  519,  518,  517,  516,  515,  514,  513,  512,
//27.0, 27.1, 27.2, 27.3, 27.4, 27.5, 27.6, 27.7, 27.8, 27.9,
   511,  510,  509,  508,  507,  506,  505,  504,  503,  502,
//28.0, 28.1, 28.2, 28.3, 28.4, 28.5, 28.6, 28.7, 28.8, 28.9, 
   501,  500,  499,  499,  498,  497,  496,  495,  494,  493,
//29.0, 29.1, 29.2, 29.3, 29.4, 29.5, 29.6, 29.7, 29.8, 29.9,
   492,  491,  490,  489,  488,  487,  486,  485,  484,  483
};
//30~34.9도
 const U16 gu16THERMISTOR_Table_30[]={ 
//30.0, 30.1, 30.2, 30.3, 30.4, 30.5, 30.6, 30.7, 30.8, 30.9,
   482,  482,  481,  480,  479,  478,  477,  476,  475,  474,
//31.0, 31.1, 31.2, 31.3, 31.4, 31.5, 31.6, 31.7, 31.8, 31.9,   
   473,  472,  471,  470,  469,  468,  468,  467,  466,  465,
//32.0, 32.1, 32.2, 32.3, 32.4, 32.5, 32.6, 32.7, 32.8, 32.9,   
   464,  463,  462,  461,  460,  459,  458,  457,  456,  456,
//33.0, 33.1, 33.2, 33.3, 33.4, 33.5, 33.6, 33.7, 33.8, 33.9,   
   455,  454,  453,  452,  451,  450,  449,  448,  447,  446,
//34.0, 34.1, 34.2, 34.3, 34.4, 34.5, 34.6, 34.7, 34.8, 34.9,   
   446,  445,  444,  443,  442,  441,  440,  439,  438,  437
};

//35~39.9도
 const U16 gu16THERMISTOR_Table_35[]={ 
//35.0, 35.1, 35.2, 35.3, 35.4, 35.5, 35.6, 35.7, 35.8, 35.9,    
   437,  436,  435,  434,  433,  432,  431,  430,  429,  428,
//36.0, 36.1, 36.2, 36.3, 36.4, 36.5, 36.6, 36.7, 36.8, 36.9,  
   428,  427,  426,  425,  424,  423,  422,  421,  421,  420,
//37.0, 37.1, 37.2, 37.3, 37.4, 37.5, 37.6, 37.7, 37.8, 37.9, 
   419,  418,  417,  416,  415,  414,  414,  413,  412,  411,
//38.0, 38.1, 38.2, 38.3, 38.4, 38.5, 38.6, 38.7, 38.8, 38.9, 
   410,  409,  408,  407,  407,  406,  405,  404,  403,  402,
//39.0, 39.1, 39.2, 39.3, 39.4, 39.5, 39.6, 39.7, 39.8, 39.9,   
   401,  401,  400,  399,  398,  397,  396,  396,  395,  394
};
//40~44.9도
 const U16 gu16THERMISTOR_Table_40[]={ 
//40.0, 40.1, 40.2, 40.3, 40.4, 40.5, 40.6, 40.7, 40.8, 40.9,
   393,  392,  391,  390,  390,  389,  388,  387,  386,  385,
//41.0, 41.1, 41.2, 41.3, 41.4, 41.5, 41.6, 41.7, 41.8, 41.9,
   385,  384,  383,  382,  381,  380,  380,  379,  378,  377,
//42.0, 42.1, 42.2, 42.3, 42.4, 42.5, 42.6, 42.7, 42.8, 42.9,
   376,  376,  375,  374,  373,  372,  371,  371,  370,  369,
//43.0, 43.1, 43.2, 43.3, 43.4, 43.5, 43.6, 43.7, 43.8, 43.9,
   368,  367,  367,  366,  365,  364,  363,  363,  362,  361,
//44.0, 44.1, 44.2, 44.3, 44.4, 44.5, 44.6, 44.7, 44.8, 44.9,   
   360,  359,  359,  358,  357,  356,  355,  355,  354, 353
};
//45~49.9도
 const U16 gu16THERMISTOR_Table_45[]={ 
//45.0, 45.1, 45.2, 45.3, 45.4, 45.5, 45.6, 45.7, 45.8, 45.9,
   352,  351,  351,  350,  349,  348,  348,  347,  346,  345,
//46.0, 46.1, 46.2, 46.3, 46.4, 46.5, 46.6, 46.7, 46.8, 46.9, 
   345,  344,  343,  342,  341,  341,  340,  339,  338,  338,
//47.0, 47.1, 47.2, 47.3, 47.4, 47.5, 47.6, 47.7, 47.8, 47.9,
   337,  336,  335,  335,  334,  333,  332,  332,  331,  330,
//48.0, 48.1, 48.2, 48.3, 48.4, 48.5, 48.6, 48.7, 48.8, 48.9, 
   329,  329,  328,  327,  326,  326,  325,  324,  323,  323,
//49.0, 49.1, 49.2, 49.3, 49.4, 49.5, 49.6, 49.7, 49.8, 49.9,
   322,  321,  320,  320,  319,  318,  318,  317,  316,  315
};

//========= 변좌 온도 Sensor Table ==================
//계산법: SAR
//SAR: Vin/(Vref+Vin)*1024+0.5의 정수값
//SK103FW
const U16 gu16SEAT_THERMISTOR_Table[]= 
{
750 ,741 ,733 ,724 ,715 ,705 ,696 ,687 ,677 ,668 ,658 ,		//0~10
     649 ,639 ,629 ,619 ,610 ,600 ,590 ,580 ,570 ,561 ,		//11~20
     551 ,541 ,531 ,522 ,512 ,502 ,493 ,483 ,474 ,465 ,		//21~30
     455 ,446 ,437 ,428 ,419 ,411 ,402 ,394 ,385 ,377 ,		//31~40
     369 ,361 ,353 ,345 ,337 ,330 ,322 ,315 ,308 ,301 ,		//41~50
     290                                                  //51 
};

typedef struct _adc_
{
    F32 Seat;           /* AD0 */
    F32 In;             /* AD1 */
    F32 Out;            /* AD2 */
    U16 EW;             /* AD3 */
    U16 Valve;          /* AD4 */
    U16 Voltage;        /* AD5 */


    /* Valve와 Voltage channel 변경 여부 확인*/
    
    U16 Delay;
} Adc_T;

Adc_T     adc;

static F32 GetWaterTemperature(U16 mu16ADTemperature);
static F32 GetSeatTemperature(U16 mu16ADSeatTemperature);

void InitADC(void)
{
    adc.Delay = ADC_DELAY;
}

void Evt_10msec_ADC_Handler(void)
{
    if ( adc.Delay != 0 )
    {
        adc.Delay--;
    }
}

U16 GetADCDelay(void)
{
    return adc.Delay;
}

U8 GetInTemperatureSection ( void )
{
    if ( adc.In < 9 )
    {
        return 0;
    }
    else if ( adc.In >= 9 
        && adc.In < 14 )
    {
        return 1;
    }
    else if ( adc.In >= 14 
        && adc.In < 20 )
    {
        return 2;
    }
    else //if ( adc.In >= 20 )
    {
        return 3;
    }	
}


F32 GetSeatSensor(void)
{
    U16 mu16SeatTemp;
    
    mu16SeatTemp = HAL_GetAdcValue( ANI_SEAT_TEMPERATURE );
    
    adc.Seat = GetSeatTemperature(mu16SeatTemp);

    return adc.Seat;
}

F32 GetInSensor(void)
{
    U16 mu16InTemp;
    
    mu16InTemp = HAL_GetAdcValue( ANI_IN_TEMPERATURE );
    
    adc.In = GetWaterTemperature(mu16InTemp);

    return adc.In;
}

F32 GetOutSensor(void)
{
    U16 mu16OutTemp;
    
    mu16OutTemp = HAL_GetAdcValue( ANI_OUT_TEMPERATURE );
    
    adc.Out = GetWaterTemperature(mu16OutTemp);

    return adc.Out;
}

U16 GetEWSensor(void)
{
    adc.EW = HAL_GetAdcValue( ANI_EW_FEEDBACK );
    return adc.EW;
}

U16 GetVoltageSensor(void)
{
    adc.Voltage = HAL_GetAdcValue( ANI_VOLTAGE_FEEDBACK );
    return adc.Voltage;
}


U16 GetValveSensor(void)
{
    adc.Valve = HAL_GetAdcValue( ANI_VALVE_FEEDBACK );
    return adc.Valve;
}




/*======================================================================*/
//          F32 GetWaterTemperature(U16 mu16ADTemperature)
//							INT_AD()의 입/출수 온도 감지 함수      			
/*======================================================================*/
static F32 GetWaterTemperature(U16 mu16ADTemperature)
{
	U8 mu16_Count=0;
	F32 mf32_ReturnTemperature=0;

	if(mu16ADTemperature >= 980 || mu16ADTemperature <= 50)   //-40이하 or 127이상
	{
		return WATER_ERROR;    //에러
	}   

	if(mu16ADTemperature >= 768 && mu16ADTemperature < 980)    //0도 ~ -40도
	{
		return WATER_LOW_TEMP;    //  0도 이하 
	}  
	if(mu16ADTemperature < 315 && mu16ADTemperature > 50)     //50도 ~ 127도
	{
		return WATER_HIGH_TEMP;    //  50도 이상
	}	
	// 0~5도
	if(mu16ADTemperature >= 724 && mu16ADTemperature <=767)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_0[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = (F32)mu16_Count * 0.1 ;
		return mf32_ReturnTemperature;
	}
	// 5~10도
	if(mu16ADTemperature >= 678 && mu16ADTemperature <=723)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_5[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 5;
		return mf32_ReturnTemperature;
	}
	// 10~15도
	if(mu16ADTemperature >= 629 && mu16ADTemperature <=677)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_10[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 10;
		return mf32_ReturnTemperature;
	}		
	//15~20도
	if(mu16ADTemperature >= 580 && mu16ADTemperature <=628)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_15[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 15;
		return mf32_ReturnTemperature;
	}
	// 20도 ~ 25도
	if(mu16ADTemperature >= 531 && mu16ADTemperature <=579)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_20[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 20;
		return mf32_ReturnTemperature;
	}
	// 25~30도
	if(mu16ADTemperature >= 483 && mu16ADTemperature <=530)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_25[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 25;
		return mf32_ReturnTemperature;
	}
	// 30~35도	
	if(mu16ADTemperature >= 437 && mu16ADTemperature <=482)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_30[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 30;
		return mf32_ReturnTemperature;
	}		
	// 35~40도	
	if(mu16ADTemperature >= 394 && mu16ADTemperature <=436)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_35[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 35;
		return mf32_ReturnTemperature;
	}		
	// 40~45도
	if(mu16ADTemperature >= 353 && mu16ADTemperature <=393)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_40[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 40;
		return mf32_ReturnTemperature;
	}			
	// 45~50도
	if(mu16ADTemperature >= 315 && mu16ADTemperature <=352)
	{
		for(mu16_Count=0;mu16_Count<50;mu16_Count++)
		{
			if(mu16ADTemperature >= gu16THERMISTOR_Table_45[mu16_Count])
				break;
		}
		mf32_ReturnTemperature = ((F32)mu16_Count * 0.1) + 45;
		return mf32_ReturnTemperature;
	}	
}
/*======================================================================*/
//          F32 GetSeatTemperature(U16 mu16ADSeatTemperature)
//							INT_AD()의 변좌온도 감지 함수   			
/*======================================================================*/
static F32 GetSeatTemperature(U16 mu16ADSeatTemperature)
{
	U8 mu16_SeatCount=0;
	F32 mf32_ReturnSeatTemperature=0;

	if(mu16ADSeatTemperature >= 974 || mu16ADSeatTemperature <= 58)   //-40도이하 or 120이상
	{
		return SEAT_ERROR;    //  에러
	}   
	
	if(mu16ADSeatTemperature > 750 && mu16ADSeatTemperature < 974)    //0~-40도
	{
		return SEAT_LOW_TEMP;    //  0도 이하
	}  
	if(mu16ADSeatTemperature < 290 && mu16ADSeatTemperature > 58)     //51 ~ 120
	{
		return SEAT_HIGH_TEMP;    //  51도 이상
	}	
  
  //정상범위
	if(mu16ADSeatTemperature >= 290 && mu16ADSeatTemperature <=750)   //0~51도
	{
		for(mu16_SeatCount=0;mu16_SeatCount<52;mu16_SeatCount++)
		{
			if(mu16ADSeatTemperature >= gu16SEAT_THERMISTOR_Table[mu16_SeatCount])
				break;
		}
		mf32_ReturnSeatTemperature = (F32)mu16_SeatCount;
		return mf32_ReturnSeatTemperature;
	}
}
