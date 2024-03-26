#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "audioMixer.h"
#include "beatMaker.h"
#include "time_helpers.h"
#include "periodTimer.h"

static pthread_t pid;

static bool stopping = false;

static Period_statistics_t audioStats;
static Period_statistics_t accelStats;

static void *textDisplayThread(void *args);

void textDisplay_init()
{
// Initialize the audio and accel statistics objects
    audioStats.numSamples = 0;
    audioStats.minPeriodInMs = 0.0;
    audioStats.maxPeriodInMs = 0.0;
    audioStats.avgPeriodInMs = 0.0;

    accelStats.numSamples = 0;
    accelStats.minPeriodInMs = 0.0;
    accelStats.maxPeriodInMs = 0.0;
    accelStats.avgPeriodInMs = 0.0;

    pthread_create(&pid,NULL,textDisplayThread,NULL);
}

static void *textDisplayThread(void *args)
{
    (void) args;
    while(!stopping) {
// Get all statistics and print them in a formatted way
        time_sleepForMs(1000);
        Period_getStatisticsAndClear(PERIOD_EVENT_AUDIO,&audioStats);
        Period_getStatisticsAndClear(PERIOD_EVENT_ACCEL, &accelStats);

        printf("M%d %3dBPM vol: %3d Audio[%0.3f, %0.3f] avg %0.3f/%d "
        "Accel[%0.3f, %0.3f] avg %0.3f/%d\n",beatMaker_getBeatMode(),
        beatMaker_getTempo(),AudioMixer_getVolume(),audioStats.minPeriodInMs,
        audioStats.maxPeriodInMs, audioStats.avgPeriodInMs,audioStats.numSamples,
        accelStats.minPeriodInMs,accelStats.maxPeriodInMs,accelStats.avgPeriodInMs,
        accelStats.numSamples);

    }
}