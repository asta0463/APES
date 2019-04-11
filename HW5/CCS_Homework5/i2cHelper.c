/*
 * i2cHelper.c
 *
 *  Created on: Apr 10, 2019
 *  Author: Ashish Tak
 */


#include "i2cHelper.h"

void I2CInit (void) {
    //Enable the I2C2 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);

    //GPIO Port N pins 4 and 5 need to be enabled and configured for I2C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    GPIOPinConfigure(GPIO_PN4_I2C2SDA);
    GPIOPinConfigure(GPIO_PN5_I2C2SCL);

    //Select the I2C function / pin type for the I2C2 pins
    GPIOPinTypeI2CSCL(GPIO_PORTN_BASE, GPIO_PIN_5);
    GPIOPinTypeI2C(GPIO_PORTN_BASE, GPIO_PIN_4);

    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), false);
}


uint16_t I2CGet2Bytes(uint8_t target_address, uint8_t register_address)
{
   //
   // Tell the master module what address it will place on the bus when
   // communicating with the slave.  Set the address to LUX_SENSOR
   // (as set in the slave module).  The receive parameter is set to false
   // which indicates the I2C Master is initiating a writes to the slave.  If
   // true, that would indicate that the I2C Master is initiating reads from
   // the slave.
   //
   I2CMasterSlaveAddrSet(I2C2_BASE, target_address, false);

   //
   // Place the data to be sent in the data register
   //
   I2CMasterDataPut(I2C2_BASE, register_address);

   //
   // Initiate send of data from the master.  Since the loopback
   // mode is enabled, the master and slave units are connected
   // allowing us to receive the same data that we sent out.
   //
   I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);

#ifdef BUSY_WAIT
   //
   // Wait until master module is says it's busy. Errata I2C#08
   //
   while(!I2CMasterBusy(I2C2_BASE));
#endif

   //
   // Wait until master module is done
   //
   while(I2CMasterBusy(I2C2_BASE));

   /*
   ** now switch to read mode
   */
   I2CMasterSlaveAddrSet(I2C2_BASE, target_address, true);

   /*
   ** read one byte
   */
   I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

#ifdef BUSY_WAIT
   //
   // Wait until master module is says it's busy. Errata I2C#08
   //
   while(!I2CMasterBusy(I2C2_BASE));
#endif

   //
   // Wait until master module is done
   //
   while(I2CMasterBusy(I2C2_BASE));

   uint32_t data_one = I2CMasterDataGet(I2C2_BASE);

   /*
   ** read one byte
   */
   I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

#ifdef BUSY_WAIT
   //
   // Wait until master module is says it's busy. Errata I2C#08
   //
   while(!I2CMasterBusy(I2C2_BASE));
#endif

   //
   // Wait until master module is done
   //
   while(I2CMasterBusy(I2C2_BASE));

   uint32_t data_two = I2CMasterDataGet(I2C2_BASE);

   return (data_one << 8) | data_two;
}

