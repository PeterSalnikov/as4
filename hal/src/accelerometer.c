#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdbool.h>
#include "../../app/include/periodTimer.h"
#include "../../app/include/time_helpers.h"
#include "../../app/include/beatLib.h"
#include "hal/accelerometer.h"

static int i2cFileDesc;

static pthread_t pid;

static bool stopping = false;

static void *airDrumThread(void *args);

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

	pthread_create(&pid,NULL,airDrumThread,NULL);
}

void i2c_cleanup()
{
	stopping = true;
	close(i2cFileDesc);
}

int initI2cBus(char* bus, int address)
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

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
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

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
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

void *airDrumThread(void *args)
{
	(void) args;

	unsigned char xL;
	unsigned char xH;

	unsigned char yL;
	unsigned char yH;

	unsigned char zL;
	unsigned char zH;

	short x;
	short y;
	short z;

	bool playX = false;
	bool playY = false;
	bool playZ = false;

	while(!stopping) {

		xL = readI2cReg(i2cFileDesc, OUT_X_L);
		xH = readI2cReg(i2cFileDesc, OUT_X_H);
		x = i2c_toShort(xL,xH);

		yL = readI2cReg(i2cFileDesc, OUT_Y_L);
		yH = readI2cReg(i2cFileDesc, OUT_Y_H);
		y = i2c_toShort(yL,yH);

		zL = readI2cReg(i2cFileDesc, OUT_Z_L);
		zH = readI2cReg(i2cFileDesc, OUT_Z_H);
		z = i2c_toShort(zL,zH);


		playX = true;
		for(int i = 0; i < 10; i++) {

			if(x < 4000) {
				playX = false;
			}

		}
		if(playX)
			AudioMixer_queueSound(beatLib_getCymbal());

		playY = true;
		for(int i = 0; i < 10; i++) {

			if(y > 100) {
				playY = false;
			}
		}
		if(playY)
			AudioMixer_queueSound(beatLib_getSnare());

		if(z > 1800) {
			AudioMixer_queueSound(beatLib_getKick());
		}

		time_sleepForMs(250);
		Period_markEvent(PERIOD_EVENT_ACCEL);

	}
}