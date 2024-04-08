
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<pthread.h>

#include "../../app/include/time_helpers.h"
#include "hal/pwm.h"
#include "hal/sharedMem-Linux.h"
// File to control PWM pin.
static bool is_initialized = false;
static bool pwm_isOn = false;

static int currentFrequency;

static pthread_t tid;

static FILE* pwm_openFile(char *filename);
static void pwm_closeFile(FILE *file);

static void pwm_setPeriod(long long val);
static void pwm_setDutyCycle(long long val);

static void playFiringSound(void);
static void playHitSound(void);
static void playMissSound(void);

static void *pwmThread(void *args);

// configures pin to PWM and has it start turned off
void pwm_init()
{
    is_initialized = true;
    system("config-pin p9.22 pwm > /dev/null");
    pwm_turnOff();
    pthread_create(&tid,NULL,&pwmThread, NULL);
}

// turn off the PWM on program exit
void pwm_cleanup()
{
    pwm_turnOff();
    is_initialized = false;
    pthread_join(tid,NULL);
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

    if(pwm_isOn == true) {
        FILE* enable = pwm_openFile("enable");
        fprintf(enable,"%d",0);
        pwm_closeFile(enable);
        pwm_isOn = false;
    }
}

// Additional sound that wasn't used
// static void playFiringSound()
// {
//     pwm_turnOn();
//     for(int i = 7040; i > 55; i -= 440) {
// 			pwm_setFrequency(i);
// 			time_sleepForMs(25);
// 		}
//     pwm_turnOff();
// }

static void playHitSound()
{
    pwm_turnOn();
    long long time = time_getTimeInMs();


    for(int c = 0; c < 8; c++) {
        for(int i = 1; i < 4186; i *= 2) {
            if(!is_initialized || (shared_isDownPressed() && time_getTimeInMs() - time > 500)) {
                pwm_turnOff();
                return;
            }
            pwm_setFrequency(i);
            time_sleepForMs(10);
        }
    }
    pwm_turnOff();
}

static void playMissSound()
{
    pwm_turnOn();
    long long time = time_getTimeInMs();
    // missed sound
    for(int c = 0; c < 4; c++) {
        for(int i = 5000; i > 100; i/= 2) {
            if(!is_initialized || (shared_isDownPressed() && time_getTimeInMs() - time > 500)) {
                pwm_turnOff();
                return;
            }
            pwm_setFrequency(i- c*75);
            time_sleepForMs(50);
        }
    }
    pwm_turnOff();
}

static void *pwmThread(void * args)
{
    (void) args;
    while(is_initialized) {
        enum State state = shared_getState();

        if(state == HIT) {
            playHitSound();
        }
        else if(state == MISS) {
            playMissSound();
        }
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

