#ifndef AUDIO_BIT_FRAME_H
#define AUDIO_BIT_FRAME_H

// Frame and header
#define HEAD_OFS 0 // HEADOFS+HEADLEN <= 64
#define HEAD_LEN 56 // HEADOFS+HEADLEN mod 8 = 0, in bits
#define FRAME_START ((HEAD_OFS+HEAD_LEN)/8)
#define HEAD_SIZE 8 // Head size in bytes

// Data and frame length
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
// Used frame modulation
#define FRAME_MOD_NRZ 0x1
#define FRAME_MOD_MAN 0x2
// Default frame lenght including HEAD + DATA + ECC + CRC
#define FRAME_DEFAULT_LEN 256

typedef struct {
  uint8_t value[FRAME_LEN_MAX];
  int length;
  unsigned char modulation;
}FrameData;

typedef struct {
  char *header; // NRZ
  char *header_mc; // Manchester2: 01->1,10->0
  char value[HEAD_LEN+1];
  int position;
  unsigned char modulation;
}FrameHead;

// Create new empty frame
FrameData NewFrameData(int frame_length, unsigned char modulation);
// Create new empty frame head
FrameHead NewFrameHead(unsigned char modulation);
//
void IncHeadPos(FrameHead *incpos);
//
int FrameHeadCompare(FrameHead head);
//
void PrintFrameData(FrameData frame);
//
void PrintFrame_STM32(FrameData frame);
//
void FrameXOR(FrameData *frame, int start);
//
void WriteFrameToFile(FrameData frame, FILE *fp);
//
uint16_t CalculateCRC16(FrameData *frame);
//
uint16_t GetFrameCRC16(FrameData frame);
//
int FrameManchesterEncode(FrameData *frame, int start);
//
int FrameManchesterDecode(FrameData *frame, int start);
// Convert 8 bits into one byte
int Bits2Byte(char bits[]);
#endif // AUDIO_BIT_FRAME_H
