/********************************************************************
  BasicDSP
  Copyright 2006-2007 
  Pieter-Tjerk de Boer PA3FWM pa3fwm@amsat.org
  Niels Moseley PE1OIT n.a.moseley@alumnus.utwente.nl
  License: GPLv2
********************************************************************/

#ifndef core_h
#define core_h

#include "wavstreamer.h"

void coremain(void);

void process_samples(short int buf_in[], short int buf_out[],int nsamp);

extern float samplerate;      // real rate at which samples are being communicated between hardware and core.c
extern float virtsamplerate;  // rate at which the user's code runs

extern float left_channel_max;
extern float right_channel_max;

#define NUMSLIDERS 5
extern float *sliderptr[NUMSLIDERS];  // we'll use [0] for the sine input frequency

extern int inputsource;
enum { INPUT_SOUND=0, INPUT_SINE, INPUT_NOISE, INPUT_QUADSINE, INPUT_WAVFILE, INPUT_IMPULSE };

extern float sweeprate;    // in Hz/sec
// note: sweep frequency in Hz is in *sliderptr[0]

extern WavStreamer wavstreamer;

extern int erroroffset1, erroroffset2;
extern char *errorstr;

int compile_one_line(const char *);
void compile_start(void);
void compile_complete(void);

void core_stop(void);

#define VAR_TO_GUI_LEN 8192
#define NUM_VAR_TO_GUI 4
extern float var_to_gui[NUM_VAR_TO_GUI][VAR_TO_GUI_LEN];
extern int var_to_gui_idx;
extern int var_to_gui_idx_prevpulse;
extern int var_to_gui_idx_prevprevpulse;
void var_to_gui_setvar(int i,const char *varname);

#ifdef WIN32
extern bool quit_flag;
#endif

#endif // Sentry

