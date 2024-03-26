/* 
Accelerometer-related functions. Includes the thread which listens for accelerometer changes to trigger 'air-drumming'
Much of the code comes from Dr Brian's I2C guide.
 */
#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define I2C_DEVICE_ADDRESS 0x18

#define WHO_AM_I 0x0F
#define CTRL_REG1 0x20

#define OUT_X_L 0x28
#define OUT_X_H 0x29

#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B

#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D

#define X_THRESH 4000
#define Y_THRESH 100
#define Z_THRESH 1800

// convert a 3-digit hex number to a short. This assumes
// that the numbers are in the correct format
void i2c_init();
short i2c_toShort(unsigned char l, unsigned char h);
// Thanks to Dr. Brian Fraser for providing i2c code
int initI2cBus(char* bus, int address);
void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

void i2c_cleanup();

#endif