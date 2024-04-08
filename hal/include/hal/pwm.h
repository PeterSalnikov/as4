#ifndef _PWM_H_
#define _PWM_H_

// Module for handling PWM signal to LED, in this context


#define PWM_DIR "/dev/bone/pwm/0/a"
#define NANOSECONDS_IN_A_SECOND 1000000000

void pwm_init(void);
void pwm_cleanup(void);

void pwm_turnOn(void);
void pwm_turnOff(void);

void pwm_setFrequency(int Hz);


#endif