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

#include "app.h"            // Contient les définitions, prototypes et structures de l'application.
#include "Mc32DriverLcd.h"  // Fournit les fonctions pour gérer l'écran LCD (initialisation, affichage, etc.).
#include "Mc32DriverAdc.h"   // Fournit les fonctions et structures pour gérer le convertisseur analogique-numérique (ADC).
#include "bsp.h"            // Inclut les fonctions spécifiques au matériel (ADC, LEDs, etc.).
#include <stdbool.h>         // Permet l'utilisation du type bool (true/false).
#include <stdint.h>          // Fournit des types standard tels que uint8_t, uint32_t, etc.


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

// Donnmée de l'app
APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Fonction callback pour le Timer 1.
 *
 * Appelée lors de chaque interruption du Timer 1. Gère un compteur pour les premières
 * secondes et lance l'exécution de tâches après ce délai.
 */
void App_Timer1Callback()
{
    static int8_t Iteration = 0;      // Compteur pour suivre les cycles (100 ms par cycle)
    static bool InitialDelayDone = false; // Indique si le délai initial de 3 secondes est terminé

    if (InitialDelayDone == false) // Si le délai initial de 3 secondes n'est pas encore terminé
    {
        Iteration++; // Incrémente le compteur à chaque cycle (100 ms)

        if (Iteration >= NBR_TIC_INIT_TIME) // Après 3 secondes (30 cycles de 100 ms)
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

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Gère l'activation d'une LED dans une séquence de chaser.
 *
 * Cette fonction éteint d'abord toutes les LEDs, puis active une LED spécifique 
 * selon la position dans le tableau `ledPins`. La LED active est déterminée par 
 * la valeur de `_chaserPosition`, qui doit être un index dans la plage [0, LED_COUNT-1].
 * Le port et la pin associés à chaque LED sont définis par les constantes spécifiques 
 * pour les pins de PORTA et PORTB.
 *
 * @param _chaserPosition L'index de la LED à activer dans le tableau `ledPins`.
 *                        La valeur doit être comprise entre 0 et `LED_COUNT - 1`.
 *
 * @note Cette fonction ne retourne aucune valeur.
 */
void chaser(uint8_t _chaserPosition)
{
    // Accéder directement à la LED correspondante
    static uint8_t ledPin ;
    // Tableau des positions des LEDs actives
    static const uint8_t ledPins[NBR_LEDS] = {RA0, RA1, RA4, RA5, RA6, RA15, RB10};
    
    // Éteindre toutes les LEDs
    TurnOffAllLEDs();

    ledPin= ledPins[_chaserPosition];

    // Si la LED est sur le PORTA (RA0 à RA7 et RA15), on l'active
    if (ledPin <= (RA6+1) || ledPin == RA15) 
    {
        LATA &= ~(1 << ledPin);  // Met le bit correspondant à 0
    }
    // Si la LED est sur le PORTB (RB10), on l'active
    else if (ledPin == RB10) 
    {
        LATB &= ~(1 << ledPin);  // Met le bit correspondant à 0
    }
}


/**
 * @brief Allume toutes les LEDs (actives bas).
 * @author LMS - VCO
 * @date 2025-01-02
 *
 * @details Cette fonction utilise les masques `LEDS_PORTA_MASK` et `LEDS_PORTB_MASK`
 *          pour forcer les broches correspondantes à l'état bas (0), allumant ainsi
 *          les LEDs connectées.
 */
void TurnOnAllLEDs(void) {
    // Allumer les LEDs sur PORTA et PORTB
    PLIB_PORTS_Write(PORTS_ID_0, PORT_CHANNEL_A, 
                     PLIB_PORTS_Read(PORTS_ID_0, PORT_CHANNEL_A) & ~LEDS_PORTA_MASK);
    PLIB_PORTS_Write(PORTS_ID_0, PORT_CHANNEL_B, 
                     PLIB_PORTS_Read(PORTS_ID_0, PORT_CHANNEL_B) & ~LEDS_PORTB_MASK);
}

/**
 * @brief Éteint toutes les LEDs (actives bas).
 * @author LMS - VCO
 * @date 2025-01-02
 *
 * @details Cette fonction utilise les masques `LEDS_PORTA_MASK` et `LEDS_PORTB_MASK`
 *          pour forcer les broches correspondantes à l'état haut (1), éteignant ainsi
 *          les LEDs connectées.
 */
void TurnOffAllLEDs(void) {
    // Éteindre les LEDs sur PORTA et PORTB
    PLIB_PORTS_Write(PORTS_ID_0, PORT_CHANNEL_A, 
                     PLIB_PORTS_Read(PORTS_ID_0, PORT_CHANNEL_A) | LEDS_PORTA_MASK);
    PLIB_PORTS_Write(PORTS_ID_0, PORT_CHANNEL_B, 
                     PLIB_PORTS_Read(PORTS_ID_0, PORT_CHANNEL_B) | LEDS_PORTB_MASK);
}


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
            TurnOnAllLEDs(); // Allume toutes les LEDs
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
                TurnOffAllLEDs(); // Éteint toutes les LEDs
                First_iteration = false; // Marque la fin de la première itération
            }
            else
            {
                chaserPosition++; // Avance d'une position dans le chaser
                if (chaserPosition == NBR_LEDS) // Si la position dépasse la dernière LED
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

/**
 * @brief Met à jour l'état actuel de l'application.
 * @author LMS
 * @date 2025-01-02
 * 
 * @param Newstate Nouveau état à affecter à l'application (type APP_STATES).
 * 
 * @details Cette fonction met à jour la variable globale `appData.state` avec
 *          la valeur de l'état fourni en paramètre.
 */ 
void APP_UpdateState (APP_STATES Newstate)
{
    appData.state = Newstate; // Met à jour l'état de l'application
}
/*******************************************************************************
 End of File
 */
