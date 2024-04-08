// #include "textDisplay.h"
// #include "udp.h"
#include "hal/display.h"
#include "time_helpers.h"
#include "hal/pwm.h"
#include "hal/accelerometer.h"
#include "hal/sharedMem-Linux.h"
#include <stdbool.h>
#include <stdio.h>
#include<unistd.h>



int main(void)
{	
	pwm_init();
	accelerometer_init();
	display_init();

	time_sleepForMs(500);
	
	shared_init();

	printf("Ready to find the dot.\n");

	shared_cleanup();
	pwm_cleanup();

	display_cleanup();
	accelerometer_cleanup();
	printf("Done!\n");
	return 0;
}
