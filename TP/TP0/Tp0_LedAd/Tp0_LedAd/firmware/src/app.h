/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>          // Fournit des types standard tels que uint8_t, uint32_t, etc.
#include <stdbool.h>         // Permet l'utilisation du type bool (true/false).
#include "system_config.h"   // Définit la configuration spécifique du système Harmony.
#include "system_definitions.h" // Contient les définitions globales et les fonctions système (timers, interruptions, etc.).
#include "Mc32DriverAdc.h"   // Fournit les fonctions et structures pour gérer le convertisseur analogique-numérique (ADC).

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END 

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define NBR_LEDS 8
// Définition des positions des LEDs
#define RA0 0
#define RA1 1
#define RA4 4
#define RA5 5
#define RA6 6
#define RA15 15
#define RB10 10

    // Masques pour les LEDs
#define LEDS_PORTA_MASK  0b1000011111110011 // RA0-RA7 et RA15
#define LEDS_PORTB_MASK  0b0000010000000000 // RB10 
 
// Nbr d'iterations de 100ms lors de l'attente post-init 
#define NBR_TIC_INIT_TIME 29
// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
	/* Application's state machine's initial state. */
	APP_STATE_INIT=0,
    APP_STATE_WAIT,        
	APP_STATE_SERVICE_TASKS,

	/* TODO: Define states used by the application state machine. */

} APP_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    S_ADCResults AdcRes;
    APP_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_DATA;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Fonction callback pour le Timer 1.
 *
 * Appelée lors de chaque interruption du Timer 1. Gère un compteur pour les premières
 * secondes et lance l'exécution de tâches après ce délai.
 */
void App_Timer1Callback(void);
	
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Initialise l'application.
 *
 * Cette fonction prépare l'application pour qu'elle puisse être exécutée.
 * Elle doit être appelée depuis la fonction `SYS_Initialize()`.
 *
 * @pre Toutes les initialisations du système doivent être terminées avant d?appeler cette fonction.
 */
void APP_Initialize(void);

/**
 * @brief Met à jour l'état courant de l'application.
 *
 * @param NewState Le nouvel état à affecter (de type @c APP_STATES).
 */
void APP_UpdateState(APP_STATES NewState);

/**
 * @brief Gère la machine à états principale de l'application.
 *
 * Cette fonction définit la logique principale de l'application et est appelée
 * de manière continue depuis `SYS_Tasks()`. En fonction de l'état courant,
 * différentes actions sont exécutées.
 */
void APP_Tasks(void);

// *****************************************************************************
// *****************************************************************************
// Section: Application specific functions
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
void chaser(uint8_t _chaserPosition);

/**
 * @brief Allume toutes les LEDs actives bas.
 *
 * Met à l'état bas toutes les broches associées aux LEDs.
 */
void TurnOnAllLEDs(void);

/**
 * @brief Éteint toutes les LEDs actives bas.
 *
 * Met à l'état haut toutes les broches associées aux LEDs.
 */
void TurnOffAllLEDs(void);
#endif /* _APP_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

