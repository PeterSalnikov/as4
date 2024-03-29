#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdbool.h>
// #include "../../app/include/periodTimer.h"
// #include "../../app/include/time_helpers.h"
// #include "../../app/include/beatLib.h"
#include "accelerometer.h"

static int i2cFileDesc;

static pthread_t pid;

static bool stopping = false;

static void *accelerometerThread(void *args);

static int initI2cBus(char* bus, int address);

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

static short x = 0;
static short y = 0;
// static short z = 0;

double accelerometer_mapCircular(short value, double min, double max);

double accelerometer_getXCoord()
{
	return accelerometer_mapCircular(x, MAPPED_COORD_MIN, MAPPED_COORD_MAX);
}

double accelerometer_getYCoord()
{
	return accelerometer_mapCircular(y, MAPPED_COORD_MIN, MAPPED_COORD_MAX);
}

short i2c_toShort(unsigned char l, unsigned char h)
{
	return (h << 4) | (l >> 4);
}

void i2c_init()
{
	system("config-pin p9_17 i2c > /dev/null");
	system("config-pin p9_18 i2c > /dev/null");

	i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	writeI2cReg(i2cFileDesc, CTRL_REG1, 0x27);

	pthread_create(&pid,NULL,accelerometerThread,NULL);
}

void i2c_cleanup()
{
	stopping = true;
	close(i2cFileDesc);
}

// Circular mapping function modified from ChatGPT
double accelerometer_mapCircular(short value, double out_min, double out_max)
{
	int range = READING_MAX - READING_MIN + 1;
	int half_range = range / 2;

	if(value - READING_MIN > half_range) {
		value -= range;
	}

	return (double)value / half_range * (out_max - out_min) / 2.0 + (out_max + out_min);
}

void *accelerometerThread(void *args)
{
	(void) args;

	unsigned char xL;
	unsigned char xH;

	unsigned char yL;
	unsigned char yH;

	// unsigned char zL;
	// unsigned char zH;


	// bool playX = false;
	// bool playY = false;
	// bool playZ = false;

	while(!stopping) {

		xL = readI2cReg(i2cFileDesc, OUT_X_L);
		xH = readI2cReg(i2cFileDesc, OUT_X_H);
		x = i2c_toShort(xL,xH);

		yL = readI2cReg(i2cFileDesc, OUT_Y_L);
		yH = readI2cReg(i2cFileDesc, OUT_Y_H);
		y = i2c_toShort(yL,yH);

		// zL = readI2cReg(i2cFileDesc, OUT_Z_L);
		// zH = readI2cReg(i2cFileDesc, OUT_Z_H);
		// z = i2c_toShort(zL,zH);

		// +/-45 degrees looks to be between the range (700,3300)
		// alternative view: horizontal is 4095. +/-45deg is also roughly 700 up and down..
		// x: longitudinal (lean)
		// y: lateral (tilt)

		// time_sleepForMs(10);
		sleep(0.01);

	}
	return NULL;
}

static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
}

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
	// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("Unable to write i2c register.");
		exit(-1);
	}

	// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(-1);
	}
	return value;
}