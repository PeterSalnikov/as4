#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include<pthread.h>
#include "../../app/include/time_helpers.h"
#include "../../pru-as4/sharedDataStruct.h"
#include "hal/accelerometer.h"
#include "hal/sharedMem-Linux.h"

/* 
Configure target pins for PRU use (depending on your GPIO needs). Must be done each boot of
the BBG.
// System calls just here for reference; they are called during initialization.
#joystick right and down, respectively
config-pin p8_15 pruin
config-pin p8_16 pruin

#neoPixel LED display
config-pin P8.11 pruout

 */
// General PRU Memomry Sharing Routine
// ----------------------------------------------------------------
#define PRU_ADDR 0x4A300000 // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN 0x80000 // Length of PRU memory
#define PRU0_DRAM 0x00000 // Offset to DRAM
#define PRU1_DRAM 0x02000
#define PRU_SHAREDMEM 0x10000 // Offset to shared memory
#define PRU_MEM_RESERVED 0x200 // Amount used by stack and heap
// Convert base address to each memory section
#define PRU0_MEM_FROM_BASE(base) ( (base) + PRU0_DRAM + PRU_MEM_RESERVED)
#define PRU1_MEM_FROM_BASE(base) ( (base) + PRU1_DRAM + PRU_MEM_RESERVED)
#define PRUSHARED_MEM_FROM_BASE(base) ( (base) + PRU_SHAREDMEM)

#define OFF 0x00000000

#define WHITE 0xffffffff

#define RED 0x000f0000
#define GREEN 0x0f000000
#define BLUE 0x00000f00

#define RED_BRIGHT 0x00ff0000
#define GREEN_BRIGHT 0xff000000
#define BLUE_BRIGHT 0x0000ff00


static uint32_t current_color = GREEN;

volatile sharedMemStruct_t *pSharedPru0;

static bool is_initialized = false;

static double curPtY;
static double curPtX;

static pthread_t tid;

static int score = 0;

static enum State state = AIMING;

// Simple function hard-coded to drive an LED based on the aim index.
// If I wanted to do animations I would just have to add an index argument.
static void driveLED(uint32_t color);

static void driveLED_all(uint32_t color);

// Return the address of the PRU's base memory
static volatile void* getPruMmapAddr(void);

static void freePruMmapAddr(volatile void* pPruBase);

static void* sharedThread(void * args);

void shared_init()
{
    is_initialized = true;

    system("config-pin p8_15 pruin > /dev/null");
	system("config-pin p8_16 pruin > /dev/null");
	system("config-pin p8.11 pruout > /dev/null");
    
    pthread_create(&tid,NULL,&sharedThread,NULL);
}

void shared_cleanup()
{
    pthread_join(tid,NULL);
    is_initialized = false;
}

int shared_getScore()
{
    return score;
}

enum State shared_getState()
{
    return state;
}

// ChatGPT RNG
double generateCoord() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Generate a random integer between 0 and RAND_MAX
    int rand_int = rand();
    
    // Convert it to a double between 0 and 1
    double rand_double = (double)rand_int / RAND_MAX;
    
    // Scale and shift it to be between -0.5 and 0.5
    return rand_double - 0.5;
}


bool shared_isDownPressed()
{
    if(is_initialized == false) {
        printf("WARNING (isDownPressed()): Shared mem uninitialized\n");
        return false;
    }
    return !pSharedPru0->isDownPressed;
}

bool shared_isRightPressed()
{
    if(is_initialized == false) {
        printf("WARNING (isRightPressed()): Shared mem uninitialized\n");
        return false;
    }
    return !pSharedPru0->isRightPressed;
}
// I think I also asked ChatGPT for this.
int mapCoordToInd(double coord)
{
    if(coord > MAPPED_COORD_MAX || coord < MAPPED_COORD_MIN) {
        return -1;
    }
    double width = 1.0 / (double)NUM_LEDS;
    return (int)((coord + 0.5) / width);
}

static double getAimY()
{
    return accelerometer_getYCoord();
}

static double getAimX()
{
    return accelerometer_getXCoord();
}

static void *sharedThread(void* args) 
{
    // Get access to shared memory
    volatile void *pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    (void) args;
    
    double prevAimY = getAimY();
    double curAimY = prevAimY;
    bool yOnTarget = false;
    curPtY = generateCoord();
    // Might have to use a separate seed for x and y.
    double prevAimX = getAimX();
    double curAimX = prevAimX;
    bool xOnTarget = false;
    curPtX = generateCoord();
    
    int consecutiveX = 0;
    int consecutiveY = 0;
    // Drive it
    while(is_initialized) {
        // Drive LED
        curAimY = getAimY() - curPtY;
        curAimX = getAimX() - curPtX;

        state = AIMING;

        if(shared_isDownPressed()) {

            if(xOnTarget && yOnTarget) {

                score++;
                printf("That's a hit! Your current score is %d\n",score);
                state = HIT;
            }

            else {
                printf("You missed!\n");
                state = MISS;
            }

            xOnTarget = false;
            yOnTarget = false;
            driveLED_all(OFF);

// X,Y generation must be separated by a short sleep since they generate based on time
// as of 04/05/2024. The sleep is also for accuracy in hit/miss readings

            curPtX = generateCoord();

            time_sleepForMs(100);
            // sleep(1);

            curPtY = generateCoord();

            printf("Fire again!\n");

        }

        if(shared_isRightPressed()) {
            driveLED_all(OFF);
            break;
        }

        // Logic for changing the color of the LED, and which are lit up.
        // Debouncing is achieved using N samples past threshold, only
        // after which the LED will change state.

        if((curAimX - prevAimX) < DELTA) {
            consecutiveX++;
        }
        else {
            consecutiveX = 0;
        }
        if(consecutiveX > SAMPLE_THRESH) {
            // if the point is to the right
            if(curAimX > DELTA) {
                current_color = GREEN;
                xOnTarget = false;
            }
            // if the point is to the left
            else if(curAimX < -1*DELTA) {
                current_color = RED;
                xOnTarget = false;
            }
            else {
                current_color = BLUE;
                xOnTarget = true;
            }
            consecutiveX = 0;
        }

        if((curAimY - prevAimY) < DELTA) {
            consecutiveY++;
        }
        else {
            consecutiveY = 0;
        }
        if(consecutiveY > SAMPLE_THRESH) {
            driveLED_all(OFF);

            if(curAimY < DELTA && curAimY > -1 * DELTA) {
                yOnTarget = true;
                driveLED_all(current_color);
            }
            else {
                driveLED(current_color);
                yOnTarget = false;
            }
            consecutiveY = 0;
        }

        prevAimX = curAimX;
        prevAimY = curAimY;
        
    }
    // Cleanup
    freePruMmapAddr(pPruBase);
}

static void driveLED(uint32_t color)
{
    int index = mapCoordToInd(getAimY() - curPtY);

    if(index < 0 || index >= NUM_LEDS) { 
        return;
    }

    pSharedPru0->colors[index] = color;
}

static void driveLED_all(uint32_t color)
{
    for(int i = 0; i < NUM_LEDS; i++) {
        pSharedPru0->colors[i] = color;
    }
}

static volatile void* getPruMmapAddr(void) 
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem");
        exit(EXIT_FAILURE);
    }
    // Points to start of PRU memory.
    volatile void* pPruBase = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE,
    MAP_SHARED, fd, PRU_ADDR);
    if (pPruBase == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return pPruBase;
}

static void freePruMmapAddr(volatile void* pPruBase)
{
    if (munmap((void*) pPruBase, PRU_LEN)) {
        perror("PRU munmap failed");
        exit(EXIT_FAILURE);
    }
}