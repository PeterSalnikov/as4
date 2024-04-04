#include "hal/display.h"
#include "hal/sharedMem-Linux.h"
#include "../../app/include/time_helpers.h"
// display_init(), writeI2cReg() courtesy of Brian Fraser's code and modified to fit use case

static pthread_t tid;

static bool is_initialized = false;

static int i2cFileDesc;

// "Dictionary" of digits, placed in numerical order for fast access
static const unsigned char a[NUM_SYMBOLS] = {209,192,152,216,200,88,88,2,216,200};
static const unsigned char b[NUM_SYMBOLS] = {165,4,131,1,34,35,163,5,163,35};

static void *display_aNumber(void *args);
static void writeI2cReg(unsigned char regAddr, unsigned char value);
static FILE* openI2cFile(char *pin);
static void closeI2cFile(FILE* file);

// system calls to configure i2c/GPIO input and I2C registers
void display_init()
{
    // system calls to configure I2C
    (void) system("config-pin p9.17 i2c > /dev/null");
    (void) system("config-pin p9.18 i2c > /dev/null");
    (void) system("echo out > /sys/class/gpio/gpio44/direction");
    (void) system("echo out > /sys/class/gpio/gpio61/direction");
    
    // Brian's code
    i2cFileDesc = open(I2CDRV_LINUX_BUS1,O_RDWR);

    int result = ioctl(i2cFileDesc, I2C_SLAVE, I2C_DEVICE_ADDRESS);
    if(result < 0) {
        perror("I2C: Unable to set I2C device to slave address.\n");
        exit(1);
    }

    writeI2cReg(REG_DIRA, 0x00);
    writeI2cReg(REG_DIRB, 0x00);

    // initialize the thread
    is_initialized = true;
    if(pthread_create(&tid,NULL, &display_aNumber, NULL) != 0) {
        perror("Error creating 14-seg display thread.\n");
        exit(1);
    }
    
}

void display_cleanup()
{
    is_initialized = false;
    if(pthread_join(tid,NULL) != 0) {
        perror("Woops! Error joining 14-segment thread\n");
        exit(1);
    }
    display_turnOffAll();
}

void display_turnOffAll()
{
    FILE* left = openI2cFile(DIGIT_LEFT);
    FILE* right = openI2cFile(DIGIT_RIGHT);
    fprintf(left,"%d",0);
    fprintf(right,"%d",0);
    closeI2cFile(left);
    closeI2cFile(right);
}

// This should really enforce only allowing the left or right digit char to be passed
void display_turnOnOne(char *whichOne)
{
    FILE* digit = openI2cFile(whichOne);
    fprintf(digit,"%d",1);
    closeI2cFile(digit);
}

// Thread function parses dip reading as needed and displays on 14-segment
static void *display_aNumber(void *args)
{
    (void) args;
    int leftDigit = 0;
    int rightDigit = 0;
    long long stopwatch = time_getTimeInMs();
    int val;

    while(is_initialized) {

        // run once a second to update val
        if(time_getTimeInMs() - stopwatch > 1000) {

            stopwatch = time_getTimeInMs();
            // val = sampler_getDipHistory();
            val = shared_getScore();

            if(val > 99) {
                leftDigit = 9;
                rightDigit = 9;
            }
            else if (val < 10) {
                leftDigit = 0;
                rightDigit = val;
            }
            else {
                // parse a two digit number to get individual digits
                while(val) {
                    rightDigit = val % 10;
                    val /= 10;
                    leftDigit = val % 10;
                    val /= 10;
                }
            }
        }
        // handle the display
        display_turnOffAll();
        display_setDigit(leftDigit);
        display_turnOnOne(DIGIT_LEFT);
        time_sleepForMs(6);
        
        display_turnOffAll();
        display_setDigit(rightDigit);
        display_turnOnOne(DIGIT_RIGHT);
        time_sleepForMs(6);

    }

    return NULL;
}

// writes to the I2C registers to set the digit to be displayed
void display_setDigit(int number)
{
    writeI2cReg(REG_OUTA, a[number]);
    writeI2cReg(REG_OUTB, b[number]);
}

static FILE* openI2cFile(char *pin)
{
    
    FILE* file = fopen(pin,"w");
    if(file == NULL) {
        perror("ERROR: openI2cFile: unable to open\n");
        exit(1);
    }
    return file;
}

static void closeI2cFile(FILE *file)
{
    if(file) {
        fclose(file);
    }
}

// Courtesy of Brian's I2C guide
static void writeI2cReg(unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);

    if(res != 2) {
        perror("I2C: Unable to write i2c register.\n");
        exit(1);
    }
}