#include "../include/beatMaker.h"
#include "../include/audioMixer.h"
#include "../include/beatLib.h"
#include<stdio.h>

#define DEFAULT_TEMPO 100
#define MIN_TEMPO 40
#define MAX_TEMPO 300

static enum Beat beatNum;

static short tempo;



static enum BeatMode beatMode;

// just for testing
// static char * beatNumNames[] = { "one","one and", "two", "two and", "three", "three and", "four", "four and" };

void beatMaker_init()
{
    beatMaker_setTempo(DEFAULT_TEMPO);
    beatNum = ONE;
    beatMode = OFF;
}

void beatMaker_cleanup()
{
    // do nothing
}

enum BeatMode beatMaker_getBeatMode()
{
    return beatMode;
}

void beatMaker_setBeatMode(enum BeatMode mode)
{
    beatMode = mode;
}

void beatMaker_incBeatMode()
{
    beatMode++;
    if(beatMode == END) {
        beatMode = OFF;
    }
}

short beatMaker_getTempo()
{
    return tempo;
}

void beatMaker_setTempo(short newTempo)
{
    if(newTempo < MIN_TEMPO || newTempo > MAX_TEMPO) {
        printf("Error: tempo must be between %dBPM and %dBPM.\n",MIN_TEMPO,MAX_TEMPO);
        return;
    }
    tempo = newTempo;
}

enum Beat getBeat()
{
    return beatNum;
}

void beatMaker_incBeat()
{
    
    beatNum++;

    if(beatNum > FOUR_AND) {
        beatNum = ONE;
    }

}

void beatMaker_playRock1()
{
    beatLib_loadRockBeat();
    // printf("%s\n",beatNumNames[beatNum]);
    switch(beatNum) {
        case ONE:
            AudioMixer_queueSound(beatLib_getKick());
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case ONE_AND:
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case TWO:
            AudioMixer_queueSound(beatLib_getSnare());
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case TWO_AND:
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case THREE:
            AudioMixer_queueSound(beatLib_getKick());
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case THREE_AND:
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case FOUR:
            AudioMixer_queueSound(beatLib_getSnare());
            AudioMixer_queueSound(beatLib_getHihat());
            break;
        case FOUR_AND:
            AudioMixer_queueSound(beatLib_getHihat());
            break;
            
    }

    beatMaker_incBeat();
    
}

void beatMaker_playRock2()
{
    // beatLib_loadRockBeat();
    // printf("%s\n",beatNumNames[beatNum]);
    switch(beatNum) {
        case ONE:
            AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());
            break;
        case ONE_AND:
            // AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getCymbal());
            // AudioMixer_queueSound(beatLib_getHihat());
            break;
        case TWO:
            AudioMixer_queueSound(beatLib_getKick());
            AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());

            break;
        case TWO_AND:
            AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());

            break;
        case THREE:
            AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());

            break;
        case THREE_AND:
            // AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            // AudioMixer_queueSound(beatLib_getCymbal());

            break;
        case FOUR:
            AudioMixer_queueSound(beatLib_getKick());
            AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());

            break;
        case FOUR_AND:
            AudioMixer_queueSound(beatLib_getKick());
            // AudioMixer_queueSound(beatLib_getSnare());
            // AudioMixer_queueSound(beatLib_getHihat());
            AudioMixer_queueSound(beatLib_getCymbal());

            break;
            
    }

    beatMaker_incBeat();
    
}