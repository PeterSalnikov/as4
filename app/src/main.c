// #include <alsa/asoundlib.h>

// #include "textDisplay.h"
// #include "periodTimer.h"
// #include "audioMixer.h"
// #include "beatMaker.h"
// #include "beatLib.h"
// #include "udp.h"
#include "hal/display.h"
#include "time_helpers.h"
#include "hal/joystick.h"
#include "hal/accelerometer.h"
#include "hal/sharedMem-Linux.h"
#include <stdbool.h>
#include <stdio.h>

// File used for play-back:
// If cross-compiling, must have this file available, via this relative path,
// on the target when the application is run. This example's Makefile copies the wave-files/
// folder along with the executable to ensure both are present.

// #define SAMPLE_RATE   44100
// #define NUM_CHANNELS  1
// #define SAMPLE_SIZE   (sizeof(short)) 	// bytes per sample

int main(void)
{
	// Period_init();
	display_init();
	// joystick_init();
	
	shared_init();
	time_sleepForMs(300);

	double randomY;

	// Generate random points
	while(!shared_isRightPressed()) {
		// printf("\n");
		// randomY = generateCoord();

		// time_sleepForMs(1000);
		// printf("%0.2f\n",randomY);


		// time_sleepForMs((int) waitFor);

	}

	// Some modules don't need cleanup
	// udp_cleanup();
	// i2c_cleanup();
	// AudioMixer_cleanup();
	// Period_cleanup();
	display_cleanup();

	printf("Done!\n");
	return 0;
}
