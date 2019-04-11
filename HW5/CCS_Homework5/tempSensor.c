/*
 * tempSensor.c
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#include "tempSensor.h"


#define SLAVE_ADDRESS_TMP102    0x48
#define TEMP_REG_ADDR   0x00

#define TEMPERATURE_THRESHOLD   27

xSemaphoreHandle tempSemaphore;
xQueueHandle xQueueTemp;
xTaskHandle alertTaskHandle;


/*
 * Read 2 bytes of temperature data from the sensor and apply appropriate conversion
 */
float readTemp(void) {
    uint16_t tempReg;
    tempReg= I2CGet2Bytes(SLAVE_ADDRESS_TMP102, TEMP_REG_ADDR);

    //High byte is read first
    //Bit 0 of second byte will always be 0 in 12-bit readings and 1 in 13-bit
    if(tempReg&0x01) // 13 bit mode
    {
        tempReg&=0xFFF8;
        tempReg>>=3;
    }
    else    // 12 bit mode
    {
        tempReg&=0xFFF0;
        tempReg>>=4;
    }

    return ((float)tempReg) * 0.0625;
}


void vTempTimerCallBack( TimerHandle_t xTempTimer ) {
    xSemaphoreGive(tempSemaphore);
}


void tempTaskCode (void * pvParameters) {
    I2CInit();
    tempLog txLog;
    while (1) {
        if( xSemaphoreTake( tempSemaphore, portMAX_DELAY ) == pdTRUE ) {
            txLog.temp= readTemp();
            if (txLog.temp>TEMPERATURE_THRESHOLD)
                xTaskNotifyGive(alertTaskHandle);
            txLog.timestamp = pdTICKS_TO_MS(xTaskGetTickCount());
            xQueueSend( xQueueTemp, ( void * ) &txLog, 0 );
        }
    }
}


void alertTaskCode (void * pvParameters) {
    //Wait until the Temperature sensing task notifies of a high temperature value
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        UARTprintf("\n\n!!!ALERT!!! Temperature is too high!\n");
    }

}
