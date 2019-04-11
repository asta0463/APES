/*
 * logger.c
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#include "logger.h"
#include "LED.h"
#include "tempSensor.h"

#define FLOAT_PRECISION_DIGITS  10

extern xQueueHandle xQueueTemp, xQueueLED;

void loggerTaskCode (void *pvParameters) {
    tempLog tempRxLog;
    LEDLog LEDRxLog;
    char floatStr[FLOAT_PRECISION_DIGITS];
    while (1) {
        if (xQueueTemp) {
             if( xQueueReceive( xQueueTemp, &tempRxLog, 0 ) ) {
                 snprintf(floatStr, FLOAT_PRECISION_DIGITS, "%f", tempRxLog.temp);
                 UARTprintf("\n\n<%d ms>\tTemperature: %s degrees\n", tempRxLog.timestamp, floatStr);
             }
        }
        if (xQueueLED) {
            if (xQueueReceive(xQueueLED, &LEDRxLog, 0)) {
                UARTprintf("\n<%d ms>\tToggle count: %d\tUser Name: %s\n", LEDRxLog.timestamp, LEDRxLog.toggleCount, LEDRxLog.name);
            }
        }
    }
}
