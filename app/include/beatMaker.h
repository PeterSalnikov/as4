#ifndef _BEAT_MAKER_H
#define _BEAT_MAKER_H


// 60 / bpm / 2
enum Beat {
	ONE,ONE_AND,
    TWO,TWO_AND,
    THREE,THREE_AND,
    FOUR,FOUR_AND,
    ALL,WHOLE,HALF
};

enum BeatMode { OFF, ROCK1, ROCK2, END };


void beatMaker_init(void);

short beatMaker_getTempo(void);
void beatMaker_setTempo(short newTempo);

void beatMaker_setBeatMode(enum BeatMode mode);
enum BeatMode beatMaker_getBeatMode(void);

void beatMaker_incBeatMode(void);

void beatMaker_incBeat();

void beatMaker_playRock1(void);
void beatMaker_playRock2(void);

#endif