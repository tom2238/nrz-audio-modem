#include <stdint.h>
#include <stdio.h>
#include "frame.h"
// NRZ
//{ 0x10, 0xB6, 0xCA, 0x11, 0x22, 0x96, 0x12, 0xF8} transmitted in frame
//{ 0x86, 0x35, 0xF4, 0x40, 0x93, 0xDF, 0x1A, 0x60} XORed in receiver
// Manchester
//{ 0x9A, 0x99, 0x99, 0x99, 0xA9, 0x6D, 0x55, 0x55} transmitted in frame
// No XORing, scrambling
FrameData NewFrameData(int frame_length, unsigned char modulation){
  FrameData newframe;
  newframe.modulation = modulation;
  if(newframe.modulation == FRAME_MOD_NRZ) {
      newframe.value[0] = 0x86;
      newframe.value[1] = 0x35;
      newframe.value[2] = 0xF4;
      newframe.value[3] = 0x40;
      newframe.value[4] = 0x93;
      newframe.value[5] = 0xDF;
      newframe.value[6] = 0x1A;
      newframe.value[7] = 0x60;
  } else if (newframe.modulation == FRAME_MOD_MAN) {
      newframe.value[0] = 0x9A;
      newframe.value[1] = 0x99;
      newframe.value[2] = 0x99;
      newframe.value[3] = 0x99;
      newframe.value[4] = 0xA9;
      newframe.value[5] = 0x6D;
      newframe.value[6] = 0x55;
      newframe.value[7] = 0x55;
  } else {
      newframe.value[0] = 0x00;
      newframe.value[1] = 0x00;
      newframe.value[2] = 0x00;
      newframe.value[3] = 0x00;
      newframe.value[4] = 0x00;
      newframe.value[5] = 0x00;
      newframe.value[6] = 0x00;
      newframe.value[7] = 0x00;
  }
  newframe.length = frame_length;

  int i;
  for(i=8;i<newframe.length;i++) {
    newframe.value[i] = 0;
  }
  return newframe;
}

FrameHead NewFrameHead(unsigned char modulation) {
  FrameHead newhead;
  newhead.modulation = modulation;
  // little endian
  // NRZ header
  newhead.header =    "0000100001101101010100111000100001000100011010010100100000011111";
  // little endian
  // Manchester header  01->1,10->0
  newhead.header_mc = "0101100110011001100110011001100110010101101101101010101010101010";
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
      if(head.modulation == FRAME_MOD_NRZ) {
        if (head.value[j] != head.header[HEAD_OFS+HEAD_LEN-1-i]) {
          break;
        }
      } else {
        if (head.value[j] != head.header_mc[HEAD_OFS+HEAD_LEN-1-i]) {
          break;
        }
      }
      j--;
      i++;
    }

    return i;
}

void PrintFrameData(FrameData frame) {
  int i;
  int lines = 0;
  uint16_t crctrs = GetFrameCRC16(frame);
  uint16_t crcrec = CalculateCRC16(&frame); // Calculate rewrite internal CRC value
  printf("%2d: ",lines);
  for(i=0;i<frame.length;i++) {
    printf("%02X ",frame.value[i]);
    if((i+1)%16==0) {
      printf("\n");
      lines++;
      printf("%2d: ",lines);
    }
  }
  if(crcrec==crctrs) {
    printf(" CRC OK ");
  }
  else {
    printf(" CRC FAIL ");
  }
  printf("CRC(rt) %x : %x\n",crcrec,crctrs);
}

void PrintFrame_STM32(FrameData frame) {
  // Calculate CRC
  uint16_t crctrs = GetFrameCRC16(frame);
  uint16_t crcrec = CalculateCRC16(&frame); // Calculate rewrite internal CRC value
  // Get values from STM32
  // Current frame number
  unsigned int stm_frame_count = 0;
  stm_frame_count = (frame.value[8]) << 8;
  stm_frame_count += (frame.value[9]) << 8;
  stm_frame_count += (frame.value[10]) << 8;
  stm_frame_count += (frame.value[11]) << 0;
  // ADC Vref voltage in milivolts
  uint16_t stm_adc_vref = 0;
  stm_adc_vref = (frame.value[12]) << 8;
  stm_adc_vref += (frame.value[13]) << 0;
  float stm_adc_vref_f = (float)(stm_adc_vref) / 1000; // In volts
  // ADC voltage on channel 0, pin PA0 in milivolts
  uint16_t stm_adc_ch0 = 0;
  stm_adc_ch0 = (frame.value[14]) << 8;
  stm_adc_ch0 += (frame.value[15]) << 0;
  float stm_adc_ch0_f = (float)(stm_adc_ch0) / 1000; // In volts
  // ADC MCU temperature in centi celsius degree
  uint16_t stm_adc_temp = 0;
  stm_adc_temp = (frame.value[16]) << 8;
  stm_adc_temp += (frame.value[17]) << 0;
  float stm_adc_temp_f = (float)(stm_adc_temp) / 100; // In deg C
  // ADC MCU supply voltage in milivolts
  uint16_t stm_adc_supply = 0;
  stm_adc_supply = (frame.value[18]) << 8;
  stm_adc_supply += (frame.value[19]) << 0;
  float stm_adc_supply_f = (float)(stm_adc_supply) / 1000; // In volts
  fprintf(stdout,"[%d], Power supply: %.3f V, Reference: %.3f V, ADC channel 0: %.3f V, MCU temperature: %.1f °C, ",stm_frame_count,stm_adc_supply_f,stm_adc_vref_f,stm_adc_ch0_f,stm_adc_temp_f);
  // Check CRC value
  if(crcrec==crctrs) {
    printf("[CRC OK]\n");
  }
  else {
    printf("[CRC FAIL]\n");
  }
}

void FrameXOR(FrameData *frame, int start) {
  const uint8_t mask[FRAME_XORMASK_LEN] = { 0x96, 0x83, 0x3E, 0x51, 0xB1, 0x49, 0x08, 0x98,
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

void WriteFrameToFile(FrameData frame, FILE *fp) {
  int i;
  for(i=FRAME_START+1;i<frame.length-CRC_SIZE;i++) {
    fwrite(&frame.value[i], 1, 1, fp);
  }
}

uint16_t CalculateCRC16(FrameData *frame) {
  // CRC-16/CCITT-FALSE
  int crc = 0xFFFF;          // initial value
  int polynomial = 0x1021;   // 0001 0000 0010 0001  (0, 5, 12)
  int i,j;
  uint8_t byte;
  for (i=frame->length-CRC_SIZE;i>FRAME_START+1;i--) {
    byte = frame->value[i-1] & 0xFF;
    for (j=0;j<8;j++) {
      uint8_t bit = ((byte >> (7-j) & 1) == 1);
      uint8_t c15 = ((crc >> 15 & 1) == 1);
      crc <<= 1;
      if (c15 ^ bit) {
        crc ^= polynomial;
      }
    }
  }
  crc &= 0xFFFF;
  frame->value[frame->length-1] = crc & 0xFF;
  frame->value[frame->length-2] = (crc >> 8) & 0xFF;
  return crc;
}

uint16_t GetFrameCRC16(FrameData frame) {
  uint16_t lsb = frame.value[frame.length-1] & 0xFF;
  uint16_t msb = (frame.value[frame.length-2] << 8) & 0xFF00;
  return lsb + msb;
  // return (frame.value[frame.length-1]) + (frame.value[frame.length-2] << 8);
}

int FrameManchesterEncode(FrameData *frame, int start) {
  int i,j;
  int ManFramePosition = FRAME_START+1;
  int ManBitPosition = 0;
  char Manbitbuf[8];
  uint8_t ManByte;
  uint8_t byte;
  uint8_t frame_bits[8];
  FrameData ManEncode = NewFrameData(frame->length*2, FRAME_MOD_MAN);
  //printf("Manchester frame len: %d\n",ManEncode.length);
  for(i=start;i<frame->length;i++) {
    byte = frame->value[i];
    for(j=0;j<8;j++) {
      frame_bits[j] = (byte >> j) & 0x01;
      // Manchester  01->1,10->0
      if(frame_bits[j] == 1) { // Bit Is 1
        Manbitbuf[ManBitPosition] = 0;
        ManBitPosition++;
        Manbitbuf[ManBitPosition] = 1;
        ManBitPosition++;
      } else { // Bit Is 0
        Manbitbuf[ManBitPosition] = 1;
        ManBitPosition++;
        Manbitbuf[ManBitPosition] = 0;
        ManBitPosition++;
      }
      if(ManBitPosition >= 8) {
        ManByte = (uint8_t)Bits2Byte(Manbitbuf);
        ManBitPosition = 0;
        ManEncode.value[ManFramePosition] = ManByte;
        ManFramePosition++;
      }
    }
  }
  // Rewrite frame
  frame->length = (frame->length*2)-HEAD_SIZE;
  frame->modulation = FRAME_MOD_MAN;
  for(i=0;i<frame->length;i++){
    frame->value[i] = ManEncode.value[i];
  }

  return 0;
}

int FrameManchesterDecode(FrameData *frame, int start) {
  int i,j;
  int DataFramePosition = start;
  int DataBitPosition = 0;
  unsigned int errorCount = 0;
  char Databitbuf[8];
  uint8_t DataByte;
  uint8_t byte;
  uint8_t frame_bits[8];
  uint8_t symbolCounter = 0;
  uint8_t symbolCode[2];
  FrameData DataDecode = NewFrameData(frame->length, FRAME_MOD_MAN);
  for(i=start;i<frame->length;i++) {
    byte = frame->value[i];
    for(j=0;j<8;j++) {
      frame_bits[j] = (byte >> j) & 0x01;
      // Manchester  01->1,10->0
      symbolCode[symbolCounter] = frame_bits[j];
      symbolCounter++;
      if(symbolCounter >= 2) {
        if(symbolCode[0]==0 && symbolCode[1]==1) { // Decoded 1
           Databitbuf[DataBitPosition] = 1;
        } else if (symbolCode[0]==1 && symbolCode[1]==0) { // Decoded 0
           Databitbuf[DataBitPosition] = 0;
        } else { // Some error, one symbol shift or isnot manchester
           Databitbuf[DataBitPosition] = 0;
           errorCount++;
        }
        DataBitPosition++;
        symbolCounter = 0;
      }
      if(DataBitPosition >= 8) {
        DataByte = (uint8_t)Bits2Byte(Databitbuf);
        DataBitPosition = 0;
        DataDecode.value[DataFramePosition] = DataByte;
        DataFramePosition++;
      }
    }
  }
  // Rewrite frame
  frame->length = (frame->length+HEAD_SIZE)/2;
  frame->modulation = FRAME_MOD_NRZ;
  for(i=0;i<frame->length;i++){
    frame->value[i] = DataDecode.value[i];
  }
  return errorCount;
}

int Bits2Byte(char bits[]) {
    int i, byteval=0, d=1;
    for (i = 0; i < 8; i++) {     // little endian
    /* for (i = 7; i >= 0; i--) { // big endian */
        if      (bits[i] == 1)  {
            byteval += d;
        }
        else if (bits[i] == 0)  {
            byteval += 0;
        }
        else {
            return 0x100;
        }
        d <<= 1;
    }
    return byteval;
}
