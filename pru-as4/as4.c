// Thanks to Dr. Brian Fraser for providing much of the boilerplate
// necessary to get NeoPixel 8-LED strip to work with PRU.
#include <stdint.h>
#include <pru_cfg.h>
#include<stdbool.h>
#include "resource_table_empty.h"
#include "sharedDataStruct.h"
// #include "hal/include/hal/sharedDataStruct.h"

#define STR_LEN         8       // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u

// P8_11 for output (on R30), PRU0
#define DATA_PIN 15       // Bit number to output on

volatile register uint32_t __R30;
volatile register uint32_t __R31;

#define THIS_PRU_DRAM 0x00000 // Address of DRAM
#define OFFSET 0x200 // Skip 0x100 for Stack,
// 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

#define JOYSTICK_RIGHT_MASK (1 << 15)
#define JOYSTICK_DOWN_MASK (1 << 14)


volatile sharedMemStruct_t *pSharedMemStruct =
    (volatile void *)THIS_PRU_DRAM_USABLE;
    
static void neoPixelRefresh(void);

void main(void)
{
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
    // could use a pins_areInitialized waiting loop?

    while(true) {

        pSharedMemStruct->isDownPressed = (__R31 & JOYSTICK_DOWN_MASK) != 0;
        pSharedMemStruct->isRightPressed = (__R31 & JOYSTICK_RIGHT_MASK) != 0;

        neoPixelRefresh();

    }

    __halt();
}

static void neoPixelRefresh()
{
    __delay_cycles(resetCycles);

    for(int j = 0; j < STR_LEN; j++) {
        for(int i = 31; i >= 0; i--) {
            if(pSharedMemStruct->colors[j] & ((uint32_t)0x1 << i)) {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(oneCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(oneCyclesOff-2);
            } else {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(zeroCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(zeroCyclesOff-2);
            }
        }
    }

    // Send Reset
    __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
    __delay_cycles(resetCycles);

}