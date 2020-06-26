#include "main.h"
#include "parsewav.h"

int findstr(char *buff, char *str, int pos) {
  int i;
  for (i = 0; i < 4; i++) {
    if (buff[(pos+i)%4] != str[i]) {
      break;
    }
  }
  return i;
}

WavFileInfo ReadWAVHeader(FILE *fp) {
    WavFileInfo wave = {0,0,0,0,0,fp,0};
    char txt[4+1] = "\0\0\0\0";
    unsigned char dat[4];
    int byte, p=0;

    if (fread(txt, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (strncmp(txt, "RIFF", 4)) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (fread(txt, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    // pos_WAVE = 8L
    if (fread(txt, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (strncmp(txt, "WAVE", 4)) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    // pos_fmt = 12L
    for (;;) {
        if ( (byte=fgetc(fp)) == EOF ) {
          wave.ret = ABIT_ERROR_WAVREADING;
          return wave;
        }
        txt[p % 4] = byte;
        p++;
        if (p==4) {
          p=0;
        }
        if (findstr(txt, "fmt ", p) == 4) {
          break;
        }
    }
    if (fread(dat, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (fread(dat, 1, 2, fp) < 2) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (fread(dat, 1, 2, fp) < 2) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    wave.channels = dat[0] + (dat[1] << 8);

    if (fread(dat, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    memcpy(&wave.sample_rate, dat, 4); //wave.sample_rate = dat[0]|(dat[1]<<8)|(dat[2]<<16)|(dat[3]<<24);

    if (fread(dat, 1, 4, fp) < 4) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    if (fread(dat, 1, 2, fp) < 2) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    //byte = dat[0] + (dat[1] << 8);

    if (fread(dat, 1, 2, fp) < 2) {
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }
    wave.bits_sample = dat[0] + (dat[1] << 8);

    // pos_dat = 36L + info
    for ( ; ; ) {
        if ( (byte=fgetc(fp)) == EOF ) {
          wave.ret = ABIT_ERROR_WAVREADING;
          return wave;
        }
        txt[p % 4] = byte;
        p++;
        if (p==4) {
          p=0;
        }
        if (findstr(txt, "data", p) == 4) {
          break;
        }
    }
    if (fread(dat, 1, 4, fp) < 4){
      wave.ret = ABIT_ERROR_WAVREADING;
      return wave;
    }

    fprintf(stderr, "sample_rate: %d\n", wave.sample_rate);
    fprintf(stderr, "bits       : %d\n", wave.bits_sample);
    fprintf(stderr, "channels   : %d\n", wave.channels);

    if ((wave.bits_sample != 8) && (wave.bits_sample != 16)){
      wave.ret = ABIT_ERROR_WAVBITS;
      return wave;
    }

    wave.samples_per_bit = wave.sample_rate/(float)(2*DATA_BAUD_RATE);

    fprintf(stderr, "samples/bit: %.2f\n", wave.samples_per_bit);

    return wave;
}
