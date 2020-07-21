#ifndef AUDIO_BIT_FRAME_H
#define AUDIO_BIT_FRAME_H

#include <stdint.h>
#include <stdio.h>

// Frame and header
#define HEAD_OFS 24 // HEADOFS+HEADLEN <= 64
#define HEAD_LEN 32 // HEADOFS+HEADLEN mod 8 = 0
#define FRAME_START ((HEAD_OFS+HEAD_LEN)/8)
#define pos_AUX       0x12B
// Data and frame length
//#define NDATA_LEN 320                    // std framelen 320
//#define XDATA_LEN 198
#define FRAME_LEN 518  // max framelen 518
// Scrambler mask length
#define FRAME_XORMASK_LEN 64
// Default data baud rate
#define DATA_BAUD_RATE 4800

typedef struct {
  uint8_t value[FRAME_LEN];
  int length;
}FrameData;

typedef struct {
  char *header;
  char value[HEAD_LEN+1];
  int position;
}FrameHead;

//
FrameData NewFrameData();
//
FrameHead NewFrameHead();
//
void IncHeadPos(FrameHead *incpos);
//
int FrameHeadCompare(FrameHead head);
//
void PrintFrameData(FrameData frame);
//
void FrameXOR(FrameData *frame, int start);
//
void WriteFrameToFile(FrameData frame, FILE *fp);

#endif // AUDIO_BIT_FRAME_H
