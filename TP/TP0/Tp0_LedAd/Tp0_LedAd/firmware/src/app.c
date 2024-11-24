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
    static bool First_iteration = true; // Indique si c'est la première itération
    static int8_t chaserPosition = 0;  // Position actuelle dans le chaser

    /* Check the application's current state. */
    switch ( appData.state)
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            lcd_init(); // Initialisation de l'écran LCD
            lcd_bl_on(); // Allume le rétroéclairage du LCD
            
            lcd_gotoxy(1,1); // Positionne le curseur à la première ligne
            printf_lcd("TP0 LED+AD 2024-25"); // Affiche un texte d'introduction
            lcd_gotoxy(1,2); // Positionne le curseur à la deuxième ligne
            printf_lcd("Mendes Leo"); // Affiche le nom de l'auteur
            
            BSP_InitADC10(); // Initialisation des ADC (convertisseurs analogiques-numériques)
            ledsON(); // Allume toutes les LEDs
            DRV_TMR0_Start(); // Démarre le timer 0 avec une période de 100 ms
            
            APP_UpdateState(APP_STATE_WAIT); // Passe à l'état WAIT
            break;
        }
        
        case APP_STATE_WAIT:
        {
            break; // Ne fait rien dans cet état, en attente d'un déclencheur
        }

        case APP_STATE_SERVICE_TASKS:
        {
            if (First_iteration == true) // Si c'est la première itération
            {
                ledsOFF(); // Éteint toutes les LEDs
                First_iteration = false; // Marque la fin de la première itération
            }
            else
            {
                chaserPosition++; // Avance d'une position dans le chaser
                if (chaserPosition == 8) // Si la position dépasse la dernière LED
                {
                    chaserPosition = 0; // Reboucle sur la première LED
                }
                chaser(chaserPosition); // Met à jour l'état des LEDs
            }
            
            appData.AdcRes = BSP_ReadAllADC(); // Lecture des résultats des ADC
            
            lcd_gotoxy(1,3); // Positionne le curseur à la troisième ligne
            printf_lcd("Ch0 %4d Ch1 %4d", appData.AdcRes.Chan0, appData.AdcRes.Chan1); // Affiche les valeurs des ADC
            
            APP_UpdateState(APP_STATE_WAIT); // Retourne à l'état WAIT
            break;
        }

        default:
        {
            break; // Défaut pour les états non gérés
        }
    }
}


 
void APP_UpdateState (APP_STATES Newstate)
{
    appData.state = Newstate; // Met à jour l'état de l'application
}


void App_Timer1Callback()
{
    static int8_t Iteration = 0;      // Compteur pour suivre les cycles (100 ms par cycle)
    static bool InitialDelayDone = false; // Indique si le délai initial de 3 secondes est terminé

    if (InitialDelayDone == false) // Si le délai initial de 3 secondes n'est pas encore terminé
    {
        Iteration++; // Incrémente le compteur à chaque cycle (100 ms)

        if (Iteration >= 30) // Après 3 secondes (30 cycles de 100 ms)
        {
            InitialDelayDone = true; // Le délai initial est terminé
            APP_UpdateState(APP_STATE_SERVICE_TASKS); // Passe à l'état APP_STATE_SERVICE_TASKS
        }
    }
    else
    {
        APP_UpdateState(APP_STATE_SERVICE_TASKS); // Passe à SERVICE_TASKS à chaque cycle après le délai initial
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

void chaser(uint8_t _chaserPosition) // Gère la position du chaser
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
            ledsOFF();// Éteint toutes les LEDs si la position est invalide
            break;
    }
}


/*******************************************************************************
 End of File
 */
