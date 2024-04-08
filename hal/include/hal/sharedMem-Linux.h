#ifndef _SHARED_MEM_LINUX_H_
#define _SHARED_MEM_LINUX_H_

#include<stdbool.h>

// Added header file to Dr. Brian Fraser's code to introduce enum for shot state and other helper functions.
// Handles primary game logic, reading of joystick state and NeoPixel LED patterns.

#define DELTA 0.1
#define SAMPLE_THRESH 150

enum State {
    AIMING,
    HIT,
    MISS
};

void shared_init(void);
void shared_cleanup(void);
// Retrieve game score
int shared_getScore(void);
// Returns AIMING, HIT, or MISS based on shot success or if a shot is not being made.
enum State shared_getState(void);

// Check if any of the joystick directions are pressed.
bool shared_isDownPressed(void);
bool shared_isRightPressed(void);

#endif