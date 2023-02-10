#include "hw.h"

#include "key_handler.h"
#include "key_common_handler.h"

#include "Buzzer.h"
#include "Keypba.h"



/*Main Function*/
static U8 SelPower ( void );

static U8 SelStop ( void );
static U8 SelClean( void );
static U8 SelBidet( void );
static U8 SelDry( void );
static U8 SelState ( void );


static U8 SelSter ( void );
static U8 SelNozzleClean ( void );

static U8 SelSeatTemp( void );
static U8 SelWaterTemp( void );

/*Press*/
static U8 SelPressBack( void );
static U8 SelPressFront( void );

/*Nozzle*/
static U8 SelNozzleBack( void );
static U8 SelNozzleFront( void );


KeyEventList_T KeyEventList[] =
{
    /* KEY,       Short Pushed,     Long Pushed 2sec, Long Pusehd 5sec, Pop,   Short off */
    /* SINGLE KEY */                                  
    { K_CLEAN,          SelClean,       NULL,       NULL,               NULL,     NULL        	},
    { K_BIDET,          SelBidet,       NULL,       NULL,               NULL,     NULL        	},
    { K_STATE,          SelState,       NULL,       NULL,               NULL,     NULL          },
    { K_STOP,           SelStop,        NULL,       NULL,               NULL,     NULL	        },
                                                       
    { K_PRESS_UP,       SelPressFront,  NULL,       NULL,               NULL,     NULL        	},
    { K_PRESS_DOWN,     SelPressBack,   NULL,       NULL,               NULL,     NULL        	},
    { K_NOZZLE_UP,      SelNozzleFront, NULL,       NULL,               NULL,     NULL        	},
    { K_NOZZLE_DOWN,    SelNozzleBack,  NULL,       NULL,               NULL,     NULL        	},

    { K_DRY,            SelDry,         NULL,       NULL,               NULL,     NULL        	},                                                        
    { K_STER,           SelSter,        NULL,       NULL,               NULL,     NULL          },
        
    { K_WATER_TEMP,     SelWaterTemp,   NULL,       NULL,               NULL,     NULL        	},
    { K_SEAT_TEMP,      SelSeatTemp,    NULL,       NULL,               NULL,     NULL        	},
                                                                         
    { K_NOZZLE_CLEAN,   SelNozzleClean, NULL,       NULL,               NULL,     NULL	        },
    { K_POWER,          SelPower,       NULL,       NULL,               NULL,     NULL          },

    /* not used */
//    { K_POWER_SAVE,     SelPowerSave,   OnPowerSave,    NULL,         NULL,     NULL        	},
//    { K_MOOD,           SelMood,        NULL,           NULL,         NULL,     NULL        	},
//    { K_MOVE,           SelMoveStart,   NULL,           NULL,         NULL,     NULL        	},

    /* Side Key */
    { K_SIDE_1,         NULL,           NULL,           NULL,         NULL,     NULL        	},
    { K_SIDE_2,         NULL,           NULL,           NULL,         NULL,     NULL        	},
    { K_SIDE_3,         NULL,           NULL,           NULL,         NULL,     NULL        	},

    /* MULTI KEY */                                     
    { K_SETTING,        NULL,           NULL,           NULL,         NULL,     NULL        	},
    { K_REMOTE_PARING,  NULL,           NULL,           NULL,         NULL,     NULL        	},
    { K_AGING,          NULL,           NULL,           NULL,         NULL,     NULL        	},
    { K_FACTORY_MODE,   NULL,           NULL,           NULL,         NULL,     NULL        	}



};


void* GetKeyEventList(void)
{
    return KeyEventList;
}

U8 GetKeyEventListSize(void)
{
    return ( sizeof(KeyEventList) / sizeof(KeyEventList_T) );
}



extern U8 gu8PopKeyStatePass;
extern U8 gu8LongKeyPower;


static U8 SelPower ( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_0 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_1 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;
} 

static U8 SelNozzleClean ( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_1 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_2 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelSeatTemp( void )
{

    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_2 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_3 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelWaterTemp( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_3 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_4 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelSter ( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_4 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_5 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelDry( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_5 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_6 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelPressBack (void)
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_6 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_7 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelNozzleBack(void)
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_7 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_8 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelPressFront(void)
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_8 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_9 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelNozzleFront(void)
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_9 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_10 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelState ( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_10 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_11 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelBidet( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_11 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_12 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;
}

static U8 SelStop ( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_12 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_13 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}

static U8 SelClean( void )
{
    U8 mu8Sound;
    
    if ( GetKeyPBAStep() == KEY_PBA_STEP_13 )
    {
        SetKeyPBAStep ( KEY_PBA_STEP_0 );
        mu8Sound = BUZZER_SELECT;
    }
    else
    {
        mu8Sound = BUZZER_WARNING;
    }
    
    return mu8Sound;

}



