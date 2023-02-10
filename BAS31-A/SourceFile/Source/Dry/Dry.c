#include "hw.h"

#include "Mode.h"
#include "Dry.h"
#include "SteppingMotor.h"
#include "Sterilize.h"

#define DRY_HEATER_PERIOD               60          /* 10msec * 60 = 600msec */

#define DRY_HEATER_ON_TIME              200         /* 10msec * 100 * 2 = 2sec*/
#define DRY_RELAY_ON_TIME               180         /* 10msec * 100 * 1.8 = 1.8sec*/

#define DRY_HEATER_OFF_TIME             20          /* 10msec * 20 = 200msec*/
#define DRY_FAN_OFF_TIME                200         /* 10msec * 100 * 2 = 2sec*/

/*���� ���� ���� ����*/
#define DRY_LEVEL_HEATER_RATE_1         (50*DRY_HEATER_PERIOD)/100      /*50%*/
#define DRY_LEVEL_HEATER_RATE_2         (65*DRY_HEATER_PERIOD)/100      /*65%*/
#define DRY_LEVEL_HEATER_RATE_3         (80*DRY_HEATER_PERIOD)/100      /*80%*/


typedef struct _dry_
{
    U8 Start;
    U8 Service;
    U8 Level;
    U16 Time;
    
    U16 RelayOnTime;
    U16 HeaterOnTime;
    
    U16 HeaterOffTime;
    U16 FanOffTime;
    
    
    U16 HeaterPeriod;
    U16 HeaterRate;
    
} Dry_T;

Dry_T    dry;

static void Dry_Timer ( void );
static void Dry_Start ( void );
static void Dry_End ( void );

void InitDry ( void )
{
    dry.Start = FALSE;
    dry.Service = FALSE;
    dry.Level = 0;
    dry.Time = 0;
    
    dry.RelayOnTime = 0;
    dry.HeaterOnTime = 0;
    dry.HeaterOffTime = 0;
    dry.FanOffTime = 0;
    
    dry.HeaterPeriod = 0;
    dry.HeaterRate = 0;
}

void SetDryStart ( U8 mu8Val )
{
    dry.Start = mu8Val;
}

U8 GetDryStart ( void )
{
    return dry.Start;
}

void SetDryService ( U8 mu8Val )
{
    dry.Service = mu8Val;
}

U8 GetDryService ( void )
{
    return dry.Service;
}

void SetDryLevel ( U8 mu8Val )
{
    dry.Level = mu8Val;
}

U8 GetDryLevel ( void )
{
    return dry.Level;
}

void SetDryTime ( U16 mu16Val )
{
    dry.Time = mu16Val;
}

U16 GetDryTime ( void )
{
    return dry.Time;
}

void Evt_10msec_Dry_Handler ( void )
{

    Dry_Timer();
    Dry_Start();
    Dry_End();
}

static void Dry_Timer ( void )
{
    if ( dry.HeaterPeriod < DRY_HEATER_PERIOD )
    {
        dry.HeaterPeriod++;
    }
    else
    {
        dry.HeaterPeriod = 0;
    }
    
    /////////////////////////////////////////////////////////////////////////////
	//Dry ���� ������
	//1. Fan : �������
	//2. Relay: Fan ON �� 1.8sec
	//3. Heater: Fan ON �� 2sec
	/////////////////////////////////////////////////////////////////////////////	
	if( GET_STATUS_FAN() == TRUE )      //FAN ON
	{
	    /*2016-03-29. ���� ����. ���� ���۽� EW ���� ����*/
	    SetSterOnOff ( FALSE );
	    
		//Fan	2��-> Heater ON	
		if ( dry.HeaterOnTime < DRY_HEATER_ON_TIME ) 
		{
			dry.HeaterOnTime++;
		}
		
		//Fan	1.8�� -> Relay ON	
		if ( dry.RelayOnTime < DRY_RELAY_ON_TIME )	
		{
			dry.RelayOnTime++;
		}
	}
	else
	{
		dry.HeaterOnTime = 0;
		dry.RelayOnTime	= 0;
	}
	
	/////////////////////////////////////////////////////////////////////////////
	//Dry ���� ������
	//1. Heater OFF
	//2. Relay: Heater OFF �� 0.2sec
	//3. FAN: Heater OFF �� 2sec
	/////////////////////////////////////////////////////////////////////////////
	if( GET_STATUS_DRY_HEATER() == FALSE )      //DRY HEATER OFF	
	{
		if ( GetDryLevel() != 0 )			//���� �µ� �����Ǿ� ������ FAN 2�� �� ����
		{
			if ( dry.HeaterOffTime < DRY_HEATER_OFF_TIME) 
			{
				dry.HeaterOffTime++;				
			}
			if ( dry.FanOffTime	< DRY_FAN_OFF_TIME )  
			{
				dry.FanOffTime++;				
			}
		}
		else			//���� �µ� ���� �ȵǾ� ������ FAN	��� OFF
		{
			dry.FanOffTime = DRY_FAN_OFF_TIME;				//FAN OFF
			dry.HeaterOffTime = DRY_HEATER_OFF_TIME;     //Heater OFF
		}
	}
	else        //���� ���� ����
	{
		dry.HeaterOffTime = 0;
		dry.FanOffTime = 0;
	}
}

static void Dry_Start ( void )
{
	if ( ( GetModeSel() == 3 ) 
	    && dry.Start == TRUE )
	{
	    TURN_ON_FAN();
        Motor_State( (U8) TYPE_MOTOR_DRY, 3, (U8) TYPE_BIT_MOTOR_DRY );
	    
	    if( dry.Service == FALSE )
	    {
  		    if( dry.Level == 0)
  		    {
  			    TURN_OFF_DRY_HEATER();
  			    TURN_OFF_DRY_RELAY();
  			    dry.HeaterOnTime = 0;
  			    dry.RelayOnTime = 0;
  		    }
  		    else
  		    {
  		        //2014-11-10 ���ѿ� 1��. �������� �����Ҷ� �¼����� OFF
			TURN_OFF_WATER_HEATER();
#if 0
	            TURN_OFF_WATER_HEATER_1();
	            TURN_OFF_WATER_HEATER_2();
#endif
  			    if ( dry.Level == 1 )
  			    {                         
  				    dry.HeaterRate = DRY_LEVEL_HEATER_RATE_1;
 			    }
      			else if ( dry.Level == 2 )
      			{                         
      				dry.HeaterRate = DRY_LEVEL_HEATER_RATE_2;
      			}
      			else
      			{                         
      				dry.HeaterRate = DRY_LEVEL_HEATER_RATE_3;
      			}
      			
      			//���� �µ��� ���� PWM ����
      		    if(dry.HeaterOnTime	>= DRY_HEATER_ON_TIME) //FAN ON 2�� �� PWM Heater ����
      		    {
      			    if(dry.HeaterPeriod <= dry.HeaterRate )
      		    	{
      		    		TURN_ON_DRY_HEATER();
      			    }
          			else
          			{
          				TURN_OFF_DRY_HEATER();
          			}
          		}
          		else
          		{
          		    TURN_OFF_DRY_HEATER();
          		}
          		
          		if(dry.RelayOnTime >=	DRY_RELAY_ON_TIME)	//FAN ON 1.8�� �� Relay ON
          		{
          			TURN_ON_DRY_RELAY();
          		}
          		else
          		{
          			TURN_OFF_DRY_RELAY();
          		}
      		}
    	}
    	else    //���� ������忡���� ���� OFF
    	{
		    TURN_OFF_DRY_HEATER();
		    if ( dry.HeaterOffTime >= DRY_HEATER_OFF_TIME )	 //	Relay OFF �� 	0.2�� �� Heater OFF
		    {
		        TURN_OFF_DRY_RELAY();
		    }
	    }
	}
}

static void Dry_End ( void )
{
	if( dry.Start == TRUE )
	{
	    if ( dry.Time != 0 )	 
	    {
	        dry.Time--;
	    }
	    
	    if ( ( GetModeSel() == TYPE_WAIT )
	    || ( GetModeSel() == TYPE_CLEAN )
	    || ( GetModeSel() == TYPE_BIDET ) 
	    || ( dry.Time == 0 ) )
    	{
    	    TURN_OFF_DRY_HEATER();
            Motor_State( (U8) TYPE_MOTOR_DRY, 0, (U8) TYPE_BIT_MOTOR_DRY );
    	    dry.Service = FALSE;       //���� ��忴�ٸ� �Ϲݸ���

	        if ( dry.HeaterOffTime >= DRY_HEATER_OFF_TIME )	 //	Heater OFF �� 	0.2�� �� Relay OFF
	        {
	            TURN_OFF_DRY_RELAY();
	        }
	        
	        if ( dry.FanOffTime	>= DRY_FAN_OFF_TIME )	 //	Relay OFF �� FAN OFF
	        {
	            TURN_OFF_FAN();
	            dry.Start = FALSE;
	        }
	            	    
            if( GetModeSel() == TYPE_DRY )
            {
               SetModeSel ( TYPE_WAIT );
            }
	    }
	}
}

