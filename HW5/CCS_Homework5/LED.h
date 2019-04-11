/*
 * LED.h
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#ifndef LED_H_
#define LED_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "timers.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <time.h>

#define STR_MAX_LEN 20


//Structure for passing logging data from the LED task
typedef struct {
    uint32_t toggleCount;
    char name[STR_MAX_LEN];
    uint32_t timestamp;
} LEDLog;


/*
 * Prototypes
 */

/*
 * LED task
 */
void LEDTaskCode (void *pvParameters);

/*
 * Timer callback to trigger the LED task
 */
void vLEDTimerCallBack( TimerHandle_t xLEDTimer );

void LEDGPIOInit(void);

#endif /* LED_H_ */
