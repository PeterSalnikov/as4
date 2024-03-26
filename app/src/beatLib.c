#include "../include/beatLib.h"
// #include "../include/audioMixer.h"

static wavedata_t kick;
static wavedata_t snare;
static wavedata_t hihat;
static wavedata_t cymbal;

wavedata_t* beatLib_getKick()
{
    return &kick;
}

wavedata_t* beatLib_getSnare()
{
    return &snare;
}

wavedata_t* beatLib_getHihat()
{
    return &hihat;
}

wavedata_t* beatLib_getCymbal()
{
    return &cymbal;
}

void beatLib_loadRockBeat()
{
    AudioMixer_readWaveFileIntoMemory(ROCK_KICK, &kick);
    AudioMixer_readWaveFileIntoMemory(ROCK_SNARE, &snare);
    AudioMixer_readWaveFileIntoMemory(ROCK_HIHAT, &hihat);
    AudioMixer_readWaveFileIntoMemory(ROCK_CYMBAL, &cymbal);
}