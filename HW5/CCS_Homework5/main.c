//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************


#include <stdbool.h>
#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "FreeRTOS.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"
//#include "driverlib/gpio.h"
//#include "driverlib/debug.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/uart.h"
//#include "driverlib/fpu.h"
//#include "timers.h"
//#include "utils/uartstdio.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"
#include <time.h>
#include "driverlib/sysctl.h"
#include "logger.h"
#include "LED.h"
#include "tempSensor.h"


//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//Task stack size specified as the number of variables
#define TASK_STACK_DEPTH    512

#define QUEUE_MAX_SIZE 10

extern xSemaphoreHandle tempSemaphore;
extern xSemaphoreHandle LEDSempahore;

extern xQueueHandle xQueueTemp, xQueueLED;

extern xTaskHandle alertTaskHandle;

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}


//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}


//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run at 12 MHz from the PLL.
    //
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    LEDGPIOInit();

    //
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    //
    ConfigureUART();

    //Turn on Lazy stacking and turn on the FPU
    FPULazyStackingEnable();
    FPUEnable();

    //
    // Print demo introduction.
    //
    UARTprintf("\n\nWelcome to EK-TM4C1294XL!\n");

    //Create the queues to send logging data from the temperature sensing and LED tasks
    xQueueTemp = xQueueCreate( QUEUE_MAX_SIZE, sizeof(tempLog) );
    if (!xQueueTemp) {
        UARTprintf("\n\nError in creating queue!\n");
    }
    xQueueLED = xQueueCreate( QUEUE_MAX_SIZE, sizeof(LEDLog) );
    if (!xQueueLED) {
        UARTprintf("\n\nError in creating queue!\n");
    }

    //Create the semaphores for the temperature sensing and LED tasks
    vSemaphoreCreateBinary(tempSemaphore);
    if (tempSemaphore == NULL) {
        UARTprintf("\n\nError in creating the semaphore for the temperature sensing task!\n");
    }
    vSemaphoreCreateBinary(LEDSempahore);
    if (LEDSempahore == NULL) {
        UARTprintf("\n\nError in creating the semaphore for the LED task!\n");
    }

    //Create the Temperature sensing, Logging and LED task (all at equal priorities except for alert task)
    if (xTaskCreate(tempTaskCode, (const portCHAR *)"TEMPERATURE_TASK", TASK_STACK_DEPTH, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE) {
        UARTprintf("\n\nError in creating the Temperature sensing task!\n");
    }
    if (xTaskCreate(loggerTaskCode, (const portCHAR *)"LOGGER_TASK", TASK_STACK_DEPTH, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE) {
        UARTprintf("\n\nError in creating the Logging task!\n");
    }
    if (xTaskCreate(LEDTaskCode, (const portCHAR *)"LOGGER_TASK", TASK_STACK_DEPTH, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE) {
        UARTprintf("\n\nError in creating the Logging task!\n");
    }
    if (xTaskCreate(alertTaskCode, (const portCHAR *)"ALERT_TASK", TASK_STACK_DEPTH, NULL, tskIDLE_PRIORITY + 2, &alertTaskHandle) != pdTRUE) {
        UARTprintf("\n\nError in creating the Alert task!\n");
    }

    //Create and start the timer
    TimerHandle_t xTempTimer = xTimerCreate("TempTimer", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, vTempTimerCallBack);
    if ( xTimerStart(xTempTimer, 0)!= pdPASS ) {
        UARTprintf("\n\nError in starting timer!\n");
    }
    TimerHandle_t xLEDTimer = xTimerCreate("LEDTimer", pdMS_TO_TICKS(100), pdTRUE, (void *) 0, vLEDTimerCallBack);
    if ( xTimerStart(xLEDTimer, 0)!= pdPASS ) {
        UARTprintf("\n\nError in starting timer!\n");
    }

    void vPortSetupTimerInterrupt( void );

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();

    while(1);
}
