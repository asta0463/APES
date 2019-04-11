/*
 * logger.h
 *
 *  Created on: Apr 9, 2019
 *  Author: Ashish Tak
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*
 * Prototypes
 */

/*
 * Task for UART Logging functionality
 */
void loggerTaskCode (void *pvParameters);

#endif /* LOGGER_H_ */
