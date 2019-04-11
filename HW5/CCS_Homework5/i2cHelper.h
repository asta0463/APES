/*
 * i2cHelper.h
 *
 *  Created on: Apr 10, 2019
 *  Author: Ashish Tak
 */

#ifndef I2CHELPER_H_
#define I2CHELPER_H_

#include "tempSensor.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"


/*
 * Prototypes
 */

/*
 * Perform the initialization routine for enabling I2C-2
 */
void I2CInit (void);

/*
 * Read 2 bytes from the I2C-2 peripheral
 */
uint16_t I2CGet2Bytes(uint8_t target_address, uint8_t register_address);


#endif /* I2CHELPER_H_ */
