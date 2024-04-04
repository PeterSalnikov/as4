#include "hal/pwm.h"
// File to control PWM pin.
static bool is_initialized = false;
static bool pwm_isOn = false;

static int currentFrequency;

static FILE* pwm_openFile(char *filename);
static void pwm_closeFile(FILE *file);

static void pwm_setPeriod(long long val);
static void pwm_setDutyCycle(long long val);

// configures pin to PWM and has it start turned off
void pwm_init()
{
    is_initialized = true;
    system("config-pin p9.22 pwm > /dev/null");
    pwm_turnOff();
}

// turn off the PWM on program exit
void pwm_cleanup()
{
    pwm_turnOff();
    is_initialized = false;
}

// converts frequency to period
void pwm_setFrequency(int freq)
{
    if(!is_initialized) {
        perror("Error: setFrequency(): PWM module not initialized\n");
        exit(1);
    }

    if(freq == 0) {
        pwm_turnOff();
        return;
    }
    if(currentFrequency == freq) {
        return;
    }
    pwm_turnOn();
    long long val = (1 / (double)freq)*NANOSECONDS_IN_A_SECOND;

    pwm_setPeriod(val);
    pwm_setDutyCycle(val / 2);
    currentFrequency = freq;
}

void pwm_turnOn()
{
    if(!is_initialized) {
        perror("Error: turnOn: PWM module not initialized\n");
        exit(1);
    }
    if(pwm_isOn == false) {
        FILE* enable = pwm_openFile("enable");
        fprintf(enable,"%d",1);
        pwm_closeFile(enable);
        pwm_isOn = true;
    }
}

void pwm_turnOff()
{
    if(!is_initialized) {
        perror("Error: turnOff: PWM module not initialized\n");
        exit(1);
    }
    if(pwm_isOn == true) {
        FILE* enable = pwm_openFile("enable");
        fprintf(enable,"%d",0);
        pwm_closeFile(enable);
        pwm_isOn = false;
    }
}
// Period, duty cycle abstracted by setFrequency
static void pwm_setPeriod(long long val)
{
    if(!is_initialized) {
        perror("Error: setPeriod: PWM module not initialized\n");
        exit(1);
    }
    FILE* period = pwm_openFile("period");
    fprintf(period, "%lld",val);
    pwm_closeFile(period);
}

static void pwm_setDutyCycle(long long val)
{
    if(!is_initialized) {
        perror("Error: setDutyCycle: PWM module not initialized\n");
        exit(1);
    }
    FILE* dutyCycle = pwm_openFile("duty_cycle");
    fprintf(dutyCycle,"%lld",val);
    pwm_closeFile(dutyCycle);
}

static FILE* pwm_openFile(char *filename)
{
    char toOpen[50];
    snprintf(toOpen, 50, "%s/%s",PWM_DIR,filename);
    FILE *file = fopen(toOpen,"w");
    if(file == NULL) {
        perror("ERROR: pwm_openFile: unable to open\n");
        exit(1);
    }
    return file;

}

static void pwm_closeFile(FILE* file)
{
    if(file) {
        fclose(file);
    }
}

