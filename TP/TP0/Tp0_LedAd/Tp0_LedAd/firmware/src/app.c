/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "bsp.h"
#include "Mc32DriverLcd.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    static bool First_iteration = true;
    static int8_t chaserPosition = 0;
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {

            // Init du LCD
            lcd_init();
            lcd_bl_on();
            // Premier affichage LCD
            lcd_gotoxy(1,1);
            printf_lcd("TP0 LED+AD 2024-25");
            lcd_gotoxy(1,2);
            printf_lcd("Mendes Leo");
            // Init des adcs
            BSP_InitADC10();
            // Allumage des LEDS
            ledsON();
            // Démarage timer 1 à 100ms 
            DRV_TMR0_Start();
            //Première entrée en mode WAIT
            appData.state = APP_STATE_WAIT;
            break;
        }
        
        case APP_STATE_WAIT:
        {
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            if(First_iteration == true)
            {
                ledsOFF();
                First_iteration = false;
            }
            else
            {
                chaserPosition++;
                if(chaserPosition == 8)
                {
                    chaserPosition = 0; 
                }
                chaser(chaserPosition);
            }
            
            // Lectrure des ADC
            appData.AdcRes = BSP_ReadAllADC();
            
            // Affichage sur LCD de la nouvelle valeur des ADCs
            lcd_gotoxy(1,3);
            printf_lcd("Ch0 %4d Ch1 %4d", appData.AdcRes.Chan0, appData.AdcRes.Chan1);
            
            APP_UpdateState(APP_STATE_WAIT);
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 
void APP_UpdateState (APP_STATES Newstate)
{
     appData.state = Newstate;
}

void App_Timer1Callback()
{
    static int8_t Iteration = 0;      // Compteur pour suivre les cycles (100 ms par cycle)
    static bool InitialDelayDone = false; // Indique si le délai initial de 3 secondes est terminé

    if (!InitialDelayDone) // Si le délai initial de 3 secondes n'est pas encore terminé
    {
        Iteration++; // Incrémente le compteur à chaque cycle (100 ms)

        if (Iteration >= 30) // Après 3 secondes (30 cycles)
        {
            InitialDelayDone = true; // Le délai initial est terminé
            APP_UpdateState(APP_STATE_SERVICE_TASKS); // Passe à l'état APP_STATE_SERVICE_TASKS
        }
    }
    else
    {
        // Après le délai initial, mettre à jour l'état à chaque cycle
        APP_UpdateState(APP_STATE_SERVICE_TASKS);
    }
}

void ledsON()
{
    BSP_LEDOn(BSP_LED_0);
    BSP_LEDOn(BSP_LED_1);
    BSP_LEDOn(BSP_LED_2);
    BSP_LEDOn(BSP_LED_3);
    BSP_LEDOn(BSP_LED_4);
    BSP_LEDOn(BSP_LED_5);    
    BSP_LEDOn(BSP_LED_6);
    BSP_LEDOn(BSP_LED_7);
}

void ledsOFF()
{
    BSP_LEDOff(BSP_LED_0);
    BSP_LEDOff(BSP_LED_1);
    BSP_LEDOff(BSP_LED_2);
    BSP_LEDOff(BSP_LED_3);
    BSP_LEDOff(BSP_LED_4);
    BSP_LEDOff(BSP_LED_5);    
    BSP_LEDOff(BSP_LED_6);
    BSP_LEDOff(BSP_LED_7);
}

void chaser(uint8_t _chaserPosition)
{
    switch(_chaserPosition)
    {
        case 0:
            LED0_W = 0; 
            LED7_W = 1;
            break;
        case 1:
            LED1_W = 0; 
            LED0_W = 1;
            break;
        case 2:
            LED2_W = 0; 
            LED1_W = 1;
            break;
        case 3:
            LED3_W = 0; 
            LED2_W = 1;
            break;
        case 4:
            LED4_W = 0; 
            LED3_W = 1;
            break;
        case 5:
            LED5_W = 0; 
            LED4_W = 1;
            break;
        case 6:
            LED6_W = 0; 
            LED5_W = 1;
            break;
        case 7:
            LED7_W = 0; 
            LED6_W = 1;
            break;
        default:
            ledsOFF();
            break;
    }
}


/*******************************************************************************
 End of File
 */
