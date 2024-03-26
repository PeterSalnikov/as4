#include "hal/joystick.h"
#include "../../app/include/audioMixer.h"
#include "../../app/include/time_helpers.h"
#include "../../app/include/beatMaker.h"
#include <pthread.h>

// Everything surrounding joystick functionality and I/O

static bool is_initialized = false;
static bool stopping = false;

static pthread_t joystickThreadId;

// Helpers to work within joystick.c
static enum Direction i2j(int joystickInd);
static FILE *joystick_openGPIOFile(int dirNum, char *toOpen, char *perm);
static void joystick_closeGPIOFile(FILE *GPIOFile);

// This is here because only joystick uses it at the moment, but with a different project should probably move it to utils
static void runCommand(char *command);

static void *joystickThread(void *args);

// i2j: helper function to convert from loop index to a joystick direction.
// The order of the directions does not matter, but...
// Not a fan of these as it is bad to scale for 8 directions. For purpose of this game is alright
static enum Direction i2j(int joystickInd)
{
    switch(joystickInd) {
        case 0:
            return UP;
        case 1:
            return DOWN;
        case 2:
            return RIGHT;
        case 3:
            return LEFT;
        case 4:
            return IN;
        default:
            return NONE;
    }
}

// fopen() and fclose() functions with parsing and error-handling built in 
static FILE *joystick_openGPIOFile(enum Direction direction, char *toOpen, char *perm)
{
    char loc[JOYSTICK_BUF];
    snprintf(loc,sizeof(loc),"%s%d/%s",JOYSTICK_PATH,direction,toOpen);

    // printf("%s",loc);

    FILE *file = fopen(loc,perm);

    if(file == NULL) {

        printf("ERROR: joystick_openFile: unable to open gpio%d. Check that the pin is exported\n",direction);
        exit(1);
    }
    return file;
}

static void joystick_closeGPIOFile(FILE *file)
{
    if(file) {
        
        fclose(file);
    }
}

// Instructor provided code for running bash scripts from C
static void runCommand(char *command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");

    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if(fgets(buffer,sizeof(buffer),pipe) == NULL)
            break;
        // printf("--> %s", buffer); //Uncomment for debugging
    }

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if(exitCode != 0) {
        perror("Unable to execute command:");
        printf("    command:    %s\n",command);
        printf("    exit code:  %d\n", exitCode);
    }
}

// Initialization: Make sure configurations are set and joystick directions set to "in"
void joystick_init()
{
    // runCommand("~/config-pins.sh");
    system("config-pin p8_47 gpio > /dev/null");
    // system("echo in > /sys/class/gpio/gpio47/direction");
    system("config-pin p8_26 gpio > /dev/null");
    // system("echo in > /sys/class/gpio/gpio26/direction");
    system("config-pin p8_46 gpio > /dev/null");
    // system("echo in > /sys/class/gpio/gpio46/direction");
    system("config-pin p8_65 gpio > /dev/null");
    // system("echo in > /sys/class/gpio/gpio65/direction");
    system("config-pin p8_27 gpio > /dev/null");
    // system("echo in > /sys/class/gpio/gpio27/direction");

    // ensure that all joystick directions are set to "in"
    for(size_t i = 0; i < NUM_JOYSTICK_DIRECTIONS; i++) {

        FILE *cur = joystick_openGPIOFile(i2j(i),"direction", "w");
        fprintf(cur, "%s","in");
        joystick_closeGPIOFile(cur);
    }

    pthread_create(&joystickThreadId, NULL, joystickThread, NULL);

    is_initialized = true;   
}

// if the joystick is pressed in any direction return true
bool joystick_isPressed()
{
    for(size_t i = 0; i < NUM_JOYSTICK_DIRECTIONS; i++) {

        FILE *cur = joystick_openGPIOFile(i2j(i),"value", "r");
        if(fgetc(cur)=='0') {

            joystick_closeGPIOFile(cur);
            return true;
        }
        joystick_closeGPIOFile(cur);
    }
    return false;
}
// If any, return the direction in which the joystick is pressed
enum Direction joystick_getCurrentDirection()
{
    if(!is_initialized) {

        printf("joystick module not initialized\n");
        exit(1);
    }

    for(size_t i = 0; i < NUM_JOYSTICK_DIRECTIONS; i++) {

        FILE *cur = joystick_openGPIOFile(i2j(i),"value", "r");
        if(fgetc(cur)=='0') {

            joystick_closeGPIOFile(cur);
            return i2j(i);
        }
        joystick_closeGPIOFile(cur);
    }

    return NONE;
}

static void setVolOrTempoOrMode()
{
        if(joystick_getCurrentDirection() == UP) {
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
        }
        if(joystick_getCurrentDirection() == DOWN) {
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
        }
        if(joystick_getCurrentDirection() == LEFT) {
            beatMaker_setTempo(beatMaker_getTempo() - 5);
        }
        if(joystick_getCurrentDirection() == RIGHT) {
            beatMaker_setTempo(beatMaker_getTempo() + 5);
        }
        if(joystick_getCurrentDirection() == IN) {
            beatMaker_incBeatMode();
        }
}

void *joystickThread(void *args)
{
    (void) args;

    while(!stopping) {

        setVolOrTempoOrMode();

        long long timeSincePressed = time_getTimeInMs();

        while(joystick_isPressed() && time_getTimeInMs() - timeSincePressed < 750) {}

        while(joystick_isPressed()) {

            setVolOrTempoOrMode();

            time_sleepForMs(250);

        }

    }

}