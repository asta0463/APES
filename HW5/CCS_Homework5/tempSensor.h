/*
 * tempSensor.h
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
//#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "timers.h"
#include "utils/uartstdio.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "i2cHelper.h"
//#include <time.h>

#define BUSY_WAIT   1


//Structure for passing logging data from the Temperature sensing task
typedef struct {
    float temp;
    uint32_t timestamp;
} tempLog;


/*
 * Prototypes
 */

/*
 * Timer callback to trigger the temperature sensing task
 */
void vTempTimerCallBack( TimerHandle_t xTempTimer );

/*
 * Task for Temperature sensing
 */
void tempTaskCode (void * pvParameters);

/*
 * Task for logging a Temperature Alert
 */
void alertTaskCode (void * pvParameters);

#endif /* TEMPSENSOR_H_ */
