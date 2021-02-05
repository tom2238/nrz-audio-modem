#ifndef AUDIO_BIT_FRAME_H
#define AUDIO_BIT_FRAME_H

// Frame and header

// *OLD //
//#define HEAD_OFS 24 // HEADOFS+HEADLEN <= 64
//#define HEAD_LEN 32 // HEADOFS+HEADLEN mod 8 = 0
//#define FRAME_START ((HEAD_OFS+HEAD_LEN)/8)

// *NEW //
#define HEAD_OFS 0 // HEADOFS+HEADLEN <= 64
#define HEAD_LEN 56 // HEADOFS+HEADLEN mod 8 = 0, in bits
#define FRAME_START ((HEAD_OFS+HEAD_LEN)/8)
#define HEAD_SIZE 8 // Head size in bytes

//#define pos_AUX       0x12B // obsolote
// Data and frame length
//#define NDATA_LEN 320                    // std framelen 320
//#define XDATA_LEN 198
#define FRAME_LEN_MAX 1024  // max framelen 1024
#define FRAME_LEN_MIN 18    // HEAD(8) + DATA(8) + ECC(0) + CRC(2)
// Scrambler mask length
#define FRAME_XORMASK_LEN 64
// Default data baud rate
#define DATA_BAUD_RATE 4800
// CRC size in bytes
#define CRC_SIZE 2
// Reed Solomon ECC size in bytes
#define ECC_SIZE 0


typedef struct {
  uint8_t value[FRAME_LEN_MAX];
  int length;
}FrameData;

typedef struct {
  char *header;
  char value[HEAD_LEN+1];
  int position;
}FrameHead;

// Create new empty frame
FrameData NewFrameData(int frame_length);
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
//
uint16_t CalculateCRC16(FrameData *frame);
//
uint16_t GetFrameCRC16(FrameData frame);
#endif // AUDIO_BIT_FRAME_H
