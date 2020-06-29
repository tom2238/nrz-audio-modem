#include "main.h"
#include "frame.h"

//{ 0x10, 0xB6, 0xCA, 0x11, 0x22, 0x96, 0x12, 0xF8} transmitted in frame
//{ 0x86, 0x35, 0xf4, 0x40, 0x93, 0xdf, 0x1a, 0x60} XORed in receiver
FrameData NewFrameData(){
  FrameData newframe;
  newframe.value[0] = 0x86;
  newframe.value[1] = 0x35;
  newframe.value[2] = 0xF4;
  newframe.value[3] = 0x40;
  newframe.value[4] = 0x93;
  newframe.value[5] = 0xDF;
  newframe.value[6] = 0x1A;
  newframe.value[7] = 0x60;
  newframe.length = FRAME_LEN;
  int i;
  for(i=8;i<newframe.length;i++) {
    newframe.value[i] = 0;
  }
  return newframe;
}

FrameHead NewFrameHead() {
  FrameHead newhead;
  newhead.header = "0000100001101101010100111000100001000100011010010100100000011111";
  newhead.position = -1;
  int i;
  for(i=0;i<HEAD_LEN+1;i++) {
    newhead.value[i] = 'x';
  }
  return newhead;
}

void IncHeadPos(FrameHead *incpos) {
  incpos->position = (incpos->position+1) % HEAD_LEN;
}

int FrameHeadCompare(FrameHead head) {
    int i = 0;
    int j = head.position;

    while (i < HEAD_LEN) {
      if (j < 0) {
        j = HEAD_LEN-1;
      }
      if (head.value[j] != head.header[HEAD_OFS+HEAD_LEN-1-i]) {
        break;
      }
      j--;
      i++;
    }

    return i;
}

void PrintFrameData(FrameData frame) {
  int i;
  int lines = 0;
  printf("%2d: ",lines);
  for(i=0;i<frame.length;i++) {
    printf("%2X ",frame.value[i]);
    if(i>=NDATA_LEN-1) { // Limit to shorter frame
      break;
    }
    if((i+1)%16==0) {
      printf("\n");
      lines++;
      printf("%2d: ",lines);
    }
  }
  printf("\n");
}

void FrameXOR(FrameData *frame, int start) {
  uint8_t mask[FRAME_XORMASK_LEN] = { 0x96, 0x83, 0x3E, 0x51, 0xB1, 0x49, 0x08, 0x98,
                                      0x32, 0x05, 0x59, 0x0E, 0xF9, 0x44, 0xC6, 0x26,
                                      0x21, 0x60, 0xC2, 0xEA, 0x79, 0x5D, 0x6D, 0xA1,
                                      0x54, 0x69, 0x47, 0x0C, 0xDC, 0xE8, 0x5C, 0xF1,
                                      0xF7, 0x76, 0x82, 0x7F, 0x07, 0x99, 0xA2, 0x2C,
                                      0x93, 0x7C, 0x30, 0x63, 0xF5, 0x10, 0x2E, 0x61,
                                      0xD0, 0xBC, 0xB4, 0xB6, 0x06, 0xAA, 0xF4, 0x23,
                                      0x78, 0x6E, 0x3B, 0xAE, 0xBF, 0x7B, 0x4C, 0xC1};
  /* LFSR: ab i=8 (mod 64):
   * m[16+i] = m[i] ^ m[i+2] ^ m[i+4] ^ m[i+6]
   * ________________3205590EF944C6262160C2EA795D6DA15469470CDCE85CF1
   * F776827F0799A22C937C3063F5102E61D0BCB4B606AAF423786E3BAEBF7B4CC196833E51B1490898
   */
   int i;
   if(start > frame->length || start < 0) {
     start = 0;
   }
   for(i=start;i<frame->length;i++) {
     frame->value[i] = frame->value[i] ^ mask[i % FRAME_XORMASK_LEN];
   }
}


