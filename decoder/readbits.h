#ifndef AUDIO_BIT_READBITS_H
#define AUDIO_BIT_READBITS_H

#include "main.h"
#include "parsewav.h"
#include "frame.h"

#define LEN_movAvg 3

typedef struct {
  int movAvg[LEN_movAvg];
  unsigned long sample_count;
  int Nvar;
  float *bufvar;
  float xsum;
  float qsum;
  float mu;
  float bvar[FRAME_LEN_MAX];
  int bitstart;
  int par;
  int par_alt;
  double bitgrenze;
  unsigned long scount;
}RBits;

//
int ReadBitsFSK(WavFileInfo wave, GetOptSettings setup, RBits *databits ,int *bit, int *len);
//
int ReadSignedSample(WavFileInfo wave, GetOptSettings setup, RBits *databits);
//
int ReadRawbit(WavFileInfo wave, GetOptSettings setup, RBits *databits,int *bit);

#endif // AUDIO_BIT_READBITS_H
