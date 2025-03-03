// Mc32Gest_RS232.C
// Canevas manipulatio TP2 RS232 SLO2 2017-18
// Fonctions d'émission et de réception des message
// CHR 20.12.2016 ajout traitement int error
// CHR 22.12.2016 evolution des marquers observation int Usart
// SCA 03.01.2018 nettoyé réponse interrupt pour ne laisser que les 3 ifs

#include <xc.h>
#include <sys/attribs.h>
#include "system_definitions.h"
// Ajout CHR
#include <GenericTypeDefs.h>
#include "app.h"
#include "GesFifoTh32.h"
#include "Mc32gest_RS232.h"
#include "gestPWM.h"
#include "Mc32CalCrc16.h"


typedef union {
        uint16_t val;
        struct {uint8_t lsb;
                uint8_t msb;} shl;
} U_manip16;


// Definition pour les messages
#define MESS_SIZE  5
#define START_BYTE  0xAA
#define CLE_CRC  0xFFFF
// Nombre de donnée concrètes dans le message
#define MESS_BODY_SIZE 2

// Emplacement du data SPEED dans le body du message
#define SPEED_LOCATION 0
// Emplacement du data ANGLE dans le body du message
#define ANGLE_LOCATION 1

// avec int8_t besoin -86 au lieu de 0xAA
#define STX_code  (-86)

// Structure décrivant le message
typedef struct {
    uint8_t Start;
    int8_t  Speed;
    int8_t  Angle;
    uint8_t MsbCrc;
    uint8_t LsbCrc;
} StruMess;


// Struct pour émission des messages
StruMess TxMess;
// Struct pour réception des messages
StruMess RxMess;

// Declaration des FIFO pour réception et émission
#define FIFO_RX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages
#define FIFO_TX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages

int8_t fifoRX[FIFO_RX_SIZE];
// Declaration du descripteur du FIFO de réception
S_fifo descrFifoRX;


int8_t fifoTX[FIFO_TX_SIZE];
// Declaration du descripteur du FIFO d'émission
S_fifo descrFifoTX;


// Initialisation de la communication sérielle
void InitFifoComm(void)
{    
    // Initialisation du fifo de réception
    InitFifo ( &descrFifoRX, FIFO_RX_SIZE, fifoRX, 0 );
    // Initialisation du fifo d'émission
    InitFifo ( &descrFifoTX, FIFO_TX_SIZE, fifoTX, 0 );
    

    
    // Init RTS 
    RS232_RTS = 1;   // interdit émission par l'autre
   
} // InitComm

 
// Valeur de retour 0  = pas de message reçu donc local (data non modifié)
// Valeur de retour 1  = message reçu donc en remote (data mis à jour)
int GetMessage(S_pwmSettings *pData)
{
    /* Utiles - Temporaires */
    commStat commStatus = ERROR_START;
    uint16_t i = 0;
    uint16_t ValCrc16 = 0;
    static uint8_t noReception = 0;
    
    /* Reception */
    uint8_t readChar = 0;
    uint8_t rxMess[MESS_BODY_SIZE];
    int8_t rxMsbCrc = 0;
    int8_t rxLsbCrc = 0;
    uint16_t rxValCrc16 = 0;
    uint16_t readSize = 0;
    
    // Traitement de réception à introduire ICI
    // Lecture et décodage fifo réception
    
    /* Obtention du caractère de start et de la taille du fifo */
    GetCharFromFifo(&descrFifoRX, &readChar);
    readSize = GetReadSize(&descrFifoRX);
    /* Si il y a au minimum le contenu de 1 message à lire */
    /* Et que le premier caracter est celui de start */
    if((readSize >= MESS_SIZE-1)&&(readChar == START_BYTE))
    {
        noReception = 0;
        /* Calcul initial du CRC */
        ValCrc16 = updateCRC16(CLE_CRC, START_BYTE);
        
        /* Tant que toutes les données CONCRETES du message ne sont pas lues */
        for(i=0; i<(MESS_BODY_SIZE); i++)
        {
            /* Lire les données du message et mettre à jour le CRC */
            GetCharFromFifo(&descrFifoRX, &rxMess[i]);
            ValCrc16 = updateCRC16(ValCrc16, rxMess[i]);
        }
        
        /* Lis le CRC MSB transmis */
        GetCharFromFifo(&descrFifoRX, &rxMsbCrc);
      
        /* Lis le CRC LSB transmis */
        GetCharFromFifo(&descrFifoRX, &rxLsbCrc);
        
        /* "fusionne" les CRC lus pour en faire un de 16bits */
        rxValCrc16 = (uint16_t)((rxMsbCrc << 8) | (rxLsbCrc & 0x00FF));
        
        /* Si le CRC correspond */
        if(rxValCrc16 == ValCrc16)
        {
            /* Met à jour les variables, vérifiées */
            /* Avec controle des signes (conversion float) */
            if(rxMess[SPEED_LOCATION] > 99)
                pData->SpeedSetting = (float)rxMess[SPEED_LOCATION] - 256;
            else
                pData->SpeedSetting = (float)rxMess[SPEED_LOCATION];
            
            if(rxMess[ANGLE_LOCATION] > 90)
                pData->AngleSetting = (float)rxMess[ANGLE_LOCATION] - 256;
            else
                pData->AngleSetting = (float)rxMess[ANGLE_LOCATION];
            
            /* Indique que la transmission s'est bien passée */
            commStatus = SUCCESS;
        }
        else
        {
            /* Indique que le CRC est faux */
            BSP_LEDToggle(BSP_LED_6);
            commStatus = ERROR_CRC;
        }
    }
    else
    {
        noReception++;
        
        if(noReception >= 10){
            
            noReception = 0;
            /* Indique que byte debut de message faux */
            commStatus = ERROR_START;
        }
    }
    
    
    // Gestion controle de flux de la réception
    if(GetWriteSpace ( &descrFifoRX) >= (2*MESS_SIZE)) {
        // autorise émission par l'autre
        RS232_RTS = 0;
    }
    return commStatus;
} // GetMessage


// Fonction d'envoi des messages, appel cyclique
void SendMessage(S_pwmSettings *pData)
{
    int8_t freeSize;
    uint16_t sendCrc = 0;
    uint8_t sendMsbCrc = 0;
    
    // Traitement émission à introduire ICI
    // Formatage message et remplissage fifo émission
    TxMess.Angle = pData->AngleSetting;
    TxMess.Speed = pData->SpeedSetting;
    TxMess.Start = START_MESS;
   
    /* Calcul du CRC */
    sendCrc =  updateCRC16(CLE_CRC, TxMess.Start);
    sendCrc =  updateCRC16(sendCrc, TxMess.Speed);
    sendCrc =  updateCRC16(sendCrc, TxMess.Angle);
    
    /* Entre le CRC */
     TxMess.MsbCrc = (sendCrc&0xFF00) >> 8;
     TxMess.LsbCrc = (sendCrc&0xFF);
    
    // TxMess.MsbCrc = updateCRC16
    
    // Tests if there are enough space in the FIFO
    freeSize = GetWriteSpace(&descrFifoTX);
    if(freeSize >= MESS_SIZE){
        
        PutCharInFifo(&descrFifoTX, TxMess.Start);
        PutCharInFifo(&descrFifoTX, TxMess.Speed);
        PutCharInFifo(&descrFifoTX, TxMess.Angle);
        PutCharInFifo(&descrFifoTX, TxMess.MsbCrc);
        PutCharInFifo(&descrFifoTX, TxMess.LsbCrc);
    }
    
    
    // Gestion du controle de flux
    // si on a un caractère à envoyer et que CTS = 0
    freeSize = GetReadSize(&descrFifoTX);
    if ((RS232_CTS == 0) && (freeSize > 0))
    {
        // Autorise int émission    
        PLIB_INT_SourceEnable(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);             
    }
}


// Interruption USART1
// !!!!!!!!
// Attention ne pas oublier de supprimer la réponse générée dans system_interrupt
// !!!!!!!!
 void __ISR(_UART_1_VECTOR, ipl5AUTO) _IntHandlerDrvUsartInstance0(void)
{
    BSP_LEDOn(BSP_LED_3);
    uint8_t freeSize, TXsize;
    int8_t chr;
    int8_t i_cts = 0;
    BOOL  TxBuffFull;
    USART_ERROR UsartStatus;    

    
    // Marque début interruption avec Led3
    LED3_W = 1;
    
    // Is this an Error interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_ERROR) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_ERROR) ) {
        /* Clear pending interrupt */
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_ERROR);
        // Traitement de l'erreur à la réception.
    }
   
    
    // Is this an RX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) ) {
                 BSP_LEDToggle(BSP_LED_4);
        // Oui Test si erreur parité ou overrun
        UsartStatus = PLIB_USART_ErrorsGet(USART_ID_1);

        if ( (UsartStatus & (USART_ERROR_PARITY |
                             USART_ERROR_FRAMING | USART_ERROR_RECEIVER_OVERRUN)) == 0) {

            // Traitement RX à faire ICI
            // Lecture des caractères depuis le buffer HW -> fifo SW
			//  (pour savoir s'il y a une data dans le buffer HW RX : PLIB_USART_ReceiverDataIsAvailable())
			//  (Lecture via fonction PLIB_USART_ReceiverByteReceive())

            // Transfert dans le FIFO de tous les chars reçus
            // 1 Si ONE_CHAR, 4 si HALF_FULL et 6 3B4FULL
            while (PLIB_USART_ReceiverDataIsAvailable(USART_ID_1))
            {
                chr = PLIB_USART_ReceiverByteReceive(USART_ID_1);
                PutCharInFifo ( &descrFifoRX, chr);
                BSP_LEDToggle(BSP_LED_5); // pour comptage
            }
            // buffer is empty, clear interrupt flag
            PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_RECEIVE);

            
                         
            LED4_W = !LED4_R; // Toggle Led4
            // buffer is empty, clear interrupt flag
            PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_RECEIVE);
        } else {
            // Suppression des erreurs
            // La lecture des erreurs les efface sauf pour overrun
            if ( (UsartStatus & USART_ERROR_RECEIVER_OVERRUN) == USART_ERROR_RECEIVER_OVERRUN) {
                   PLIB_USART_ReceiverOverrunErrorClear(USART_ID_1);
            }
        }

        
        // Traitement controle de flux reception à faire ICI
        // Gerer sortie RS232_RTS en fonction de place dispo dans fifo reception
        // ...

        
    } // end if RX

    
    // Is this an TX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) ) {
        BSP_LEDToggle(BSP_LED_4);
        // Traitement TX à faire ICI
        
        TXsize = GetReadSize (&descrFifoTX);
       // i_cts = input(RS232_CTS);
       // On vérifie 3 conditions :
       // Si CTS = 0 (autorisation d'émettre)
       // Si il y a un caractèreà émettre
       // Si le txreg est bien disponible
        
       // Il est possible de déposer un caractère
       // tant que le tampon n'est pas plein
       TxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_1);
       
        if ( (i_cts == 0) && ( TXsize > 0 ) && (TxBuffFull == false)){
            do {
                GetCharFromFifo(&descrFifoTX, &chr);
                PLIB_USART_TransmitterByteSend(USART_ID_1, chr);
                i_cts = RS232_CTS;
                TXsize = GetReadSize (&descrFifoTX);
                TxBuffFull =PLIB_USART_TransmitterBufferIsFull(USART_ID_1);
            } while ( (i_cts == 0) && ( TXsize > 0 ) &&TxBuffFull==false );
        }
        
       
       
        LED5_W = !LED5_R; // Toggle Led5
		
        // disable TX interrupt (pour éviter une interrupt. inutile si plus rien à transmettre)
        PLIB_INT_SourceDisable(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);
        
        // Clear the TX interrupt Flag (Seulement apres TX) 
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);
    }
    // Marque fin interruption avec Led3
    LED3_W = 0;
    
    BSP_LEDOff(BSP_LED_3);
 }




