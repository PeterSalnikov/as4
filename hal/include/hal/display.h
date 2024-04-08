#ifndef _DISPLAY_H_
#define _DISPLAY_H_

// 14-segment display module: handles everything related to the display

// i2c related register values
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x02
#define REG_DIRB 0x03
#define REG_OUTA 0x00
#define REG_OUTB 0x01

#define NUM_SYMBOLS 10
// #define DIGIT_LEFT "/sys/class/gpio/gpio61/value"
// #define DIGIT_RIGHT "/sys/class/gpio/gpio44/value"
// For upside down (as4)
#define DIGIT_RIGHT "/sys/class/gpio/gpio61/value"
#define DIGIT_LEFT "/sys/class/gpio/gpio44/value"

void display_init(void);
void display_cleanup(void);
// turn off both display digits
void display_turnOffAll(void);
// set the A and B OUT dirs
void display_setDigit(int number);

#endif