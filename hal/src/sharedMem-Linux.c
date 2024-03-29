#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "hal/sharedDataStruct.h"

#include<pthread.h>
#include "hal/accelerometer.h"
#include "hal/sharedMem-Linux.h"

/* 
Configure target pins for PRU use (depending on your GPIO needs). Must be done each boot of
the BBG.

#joystick right and down, respectively
config-pin p8_15 pruin
config-pin p8_16 pruin

#neoPixel LED display
config-pin P8.11 pruout

config-pin p8_12 pruout
config-pin P9_18 i2c
config-pin P9_17 i2c
i2cset -y 1 0x20 0x02 0x00
i2cset -y 1 0x20 0x03 0x00
i2cset -y 1 0x20 0x00 0x0f
i2cset -y 1 0x20 0x01 0x5e

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

#define RED 0x000f0000
#define GREEN 0x0f000000
#define BLUE 0x00000f00

#define RED_BRIGHT 0x00ff0000
#define GREEN_BRIGHT 0xff000000
#define BLUE_BRIGHT 0x0000ff00

// **For reference: my LED strip's 0-indexed LED is the one nearest the plug.
// This I will have to do in the linux portion of the code.
// w.r.t. where the target is..
// enum Aim {
//     WAY_ABOVE = {1,0,0,0,0,0,0,0},
//     TWO_ABOVE = {0,1,0,0,0,0,0,0},
//     ONE_ABOVE = {0,0,1,0,0,0,0,0},
//     BANG_ON = {1,1,1,1,1,1,1,1},
//     ONE_BELOW = {0,0,0,0,0,1,0,0},
//     TWO_BELOW = {0,0,0,0,0,0,1,0},
//     WAY_BELOW = {0,0,0,0,0,0,0,1},
//     GIVE_UP = {0,0,0,0,0,0,0,0},
// };
// ...or, I just need some index checking. It will require switch case anyhow


int mapCoordToInd(double coord)
{
    if(coord > MAPPED_COORD_MAX || coord < MAPPED_COORD_MIN) {
        return -1;
    }
    double width = 1.0 / (double)NUM_LEDS;
    return (int)((coord + 0.5) / width);
}

static int getAimY()
{
    return mapCoordToInd(accelerometer_getYCoord());
}

static void driveLED(uint32_t color);

static void driveLED_all(uint32_t color);

// Return the address of the PRU's base memory
static volatile void* getPruMmapAddr(void);

static void freePruMmapAddr(volatile void* pPruBase);

static uint32_t current_color = GREEN;

volatile sharedMemStruct_t *pSharedPru0;
// volatile sharedMemStruct_t *pSharedPru0;


static pthread_t pid;
void* sharedThread(void * args);

// static volatile void *pPruBase;

void shared_init()
{
    pthread_create(&pid,NULL,&sharedThread,NULL);
}

void *sharedThread(void* args) 
{
    volatile void *pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    (void) args;
    i2c_init();
    printf("Sharing memory with PRU\n");
    
    // Get access to shared memory for my uses

    int prevAimY = getAimY();
    int curAimY = getAimY();
    driveLED(current_color);
    bool hasChanged = false;
    int consecutiveY = 0;
    // Drive it
    for (int i = 0; i < 200000; i++) {
    // driveLED_all(0x00000000);
	// printf("HELLO\n");

        // Drive LED

        // condition for checking aim;
        // create an inner condition
        // for when the point is aiming
        // directly at the target
        // if(prevAim != getAim()) {
        curAimY = getAimY();


        if(!pSharedPru0->isDownPressed) {
            printf("down is pressed\n");
            driveLED_all(OFF);

        }
        else if(!pSharedPru0->isRightPressed) {
            printf("right is pressed\n");
        }
        else {
            if(curAimY == prevAimY) {
                consecutiveY++;
            }
            else {
                consecutiveY = 0;
            }
            if(consecutiveY > 75) {
                driveLED_all(OFF);
                driveLED(current_color);
                consecutiveY = 0;
            }
        }
        printf("current index: %d\n",getAimY());
        // printf("LEDS: %8x\n",pSharedPru0->colors[0]);
        // Timing
        prevAimY = curAimY;
        // sleep(0.001);
        
    }
    // Cleanup
    freePruMmapAddr(pPruBase);
}

// int main(void) 
// {
//     i2c_init();
//     printf("Sharing memory with PRU\n");
    
//     // Get access to shared memory for my uses
//     volatile void *pPruBase = getPruMmapAddr();
//     pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);

//     int prevAimY = getAimY();
//     int curAimY = getAimY();
//     driveLED(current_color);
//     bool hasChanged = false;
//     int consecutiveY = 0;
//     // Drive it
//     for (int i = 0; i < 200000; i++) {
//         // Drive LED

//         // condition for checking aim;
//         // create an inner condition
//         // for when the point is aiming
//         // directly at the target
//         // if(prevAim != getAim()) {
//         curAimY = getAimY();
//         if(curAimY == prevAimY) {
//             consecutiveY++;
//         }
//         else {
//             consecutiveY = 0;
//         }
//         if(consecutiveY > 75) {
//             driveLED_all(OFF);
//             driveLED(current_color);
//             consecutiveY = 0;
//         }

//         if(!pSharedPru0->isDownPressed) {
//             printf("down is pressed\n");
//             driveLED_all(OFF);

//         }
//         else if(!pSharedPru0->isRightPressed) {
//             printf("right is pressed\n");
//         }
//         else {
//             // for(int i = 0; i < NUM_LEDS; i++) {

//             //     pSharedPru0->colors[i] = 0x00000000;
//             // }
//         }
//         printf("current index: %d\n",getAimY());
//         // printf("LEDS: %8x\n",pSharedPru0->colors[0]);
//         // Timing
//         prevAimY = curAimY;
//         // sleep(0.001);
        
//     }
//     driveLED_all(0x00000000);
//     // Cleanup
//     freePruMmapAddr(pPruBase);
// }

static void driveLED(uint32_t color)
{
    pSharedPru0->colors[getAimY()] = color;
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