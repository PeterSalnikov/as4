
// #include "textDisplay.h"
// #include "udp.h"
#include "hal/display.h"
#include "time_helpers.h"
#include "hal/pwm.h"
#include "hal/accelerometer.h"
#include "hal/sharedMem-Linux.h"
#include <stdbool.h>
#include <stdio.h>


int main(void)
{
	display_init();
	shared_init();
	pwm_init();

	time_sleepForMs(300);

	// Generate random points
	while(!shared_isRightPressed()) {

		pwm_turnOn();
		// missed sound
		// for(int c = 0; c < 4; c++) {
		// 	for(int i = 5000; i > 100; i/= 2) {
		// 		pwm_setFrequency(i- c*75);
		// 		printf("%d\n",i);
		// 		time_sleepForMs(50);
		// 	}
		// }
		for(int i = 7040; i > 55; i -= 440) {
			pwm_setFrequency(i);
			time_sleepForMs(25);
		}
		pwm_turnOff();
		time_sleepForMs(200);
		// hit sound
		for(int c = 0; c < 8; c++) {
			for(int i = 1; i < 4186; i *= 2) {
				pwm_setFrequency(i);
				time_sleepForMs(10);
			}
		}



		// for(int i = 1; i < 4978; i *= 2) {
		// 	pwm_setFrequency(i);
		// 	time_sleepForMs(25);
		// }
		// for(int i = 1; i < 4699; i *= 2) {
		// 	pwm_setFrequency(i);
		// 	time_sleepForMs(25);
		// }
		// // time_sleepForMs(300);
		// // pwm_setFrequency(4699);
		// time_sleepForMs(300);
		// pwm_setFrequency(4434);
		// time_sleepForMs(300);
		// pwm_setFrequency(4186);
		// time_sleepForMs(300);
		pwm_turnOff();
		// pwm_setFrequency(4186);
		// pwm_turnOn();

		// time_sleepForMs(1000);
		// pwm_setFrequency(4435);
		// pwm_turnOn();

		time_sleepForMs(1000);

		pwm_turnOff();
	}

	// Some modules don't need cleanup
	// i2c_cleanup();
	// AudioMixer_cleanup();
	// Period_cleanup();
	display_cleanup();
	pwm_cleanup();
	printf("Done!\n");
	return 0;
}
