/*
 * LED.c
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#include "LED.h"

#define LED1_PORT   GPIO_PORTN_BASE
#define LED2_PORT   GPIO_PORTN_BASE
#define LED1_PIN    GPIO_PIN_0
#define LED2_PIN    GPIO_PIN_1

xSemaphoreHandle LEDSempahore;
xQueueHandle xQueueLED;


void LEDGPIOInit(void) {
    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    //
    // Enable the GPIO pins for the LEDs (PN0 and PN1).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(LED1_PORT, LED1_PIN);
    GPIOPinTypeGPIOOutput(LED2_PORT, LED2_PIN);

    //
    // Turn on the LEDs.
    //
    GPIOPinWrite(LED1_PORT, LED1_PIN, LED1_PIN);
    GPIOPinWrite(LED2_PORT, LED2_PIN, LED2_PIN);
}


void vLEDTimerCallBack( TimerHandle_t xLEDTimer ) {
    xSemaphoreGive(LEDSempahore);
}


void LEDTaskCode (void *pvParameters) {
    uint8_t LED1val=0, LED2val=LED2_PIN;
    LEDLog txLog;
    txLog.toggleCount=0;
    sprintf(txLog.name,"Ashish Tak");
    while (1) {
        if( xSemaphoreTake( LEDSempahore, portMAX_DELAY ) == pdTRUE ) {
            GPIOPinWrite(LED1_PORT, LED1_PIN, LED1val);
            GPIOPinWrite(LED2_PORT, LED2_PIN, LED2val);
            LED1val^=LED1_PIN;
            LED2val^=LED2_PIN;
            ++txLog.toggleCount;
            txLog.timestamp = pdTICKS_TO_MS(xTaskGetTickCount());
            xQueueSend( xQueueLED, ( void * ) &txLog, 0 );
        }
    }
}
