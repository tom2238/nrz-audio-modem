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

WavFileInfo ReadWAVHeader(FILE *fp, int baudrate) {
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

    wave.samples_per_bit = wave.sample_rate/(float)(baudrate);

    fprintf(stderr, "samples/bit: %.2f\n", wave.samples_per_bit);

    return wave;
}

unsigned int WriteWAVHeader(double tdur, WavFileInfo wavefile) {
   unsigned int numsa = (unsigned int)(tdur*wavefile.sample_rate);   // overall number of samples
   unsigned int subchunk2 = (numsa * wavefile.channels * wavefile.bits_sample)/8; //
   unsigned int blockAlign = (wavefile.channels * wavefile.bits_sample)/8;      //block size
   unsigned int byteRate = (wavefile.sample_rate * wavefile.channels * wavefile.bits_sample)/8; //byte rate per sample
   // chunk id RIFF
   fwrite("RIFF", 4, 1, wavefile.fp);
   // chunk size
   fwrite_int(36 + subchunk2, 4, wavefile.fp);
   // spec. RIFF form for WAV
   fwrite("WAVE", 4, 1, wavefile.fp);
   // subchunk1 id  format description
   fwrite("fmt ", 4, 1, wavefile.fp);
   // subchunk1 size: 16 for PCM
   fwrite_int(16, 4, wavefile.fp);
   // audio_format: 1 = PCM
   fwrite_int(1, 2, wavefile.fp);
   // channels: mono
   fwrite_int(wavefile.channels, 2, wavefile.fp);
   // sample rate
   fwrite_int(wavefile.sample_rate, 4, wavefile.fp);
   // byte rate
   fwrite_int(byteRate, 4, wavefile.fp);
   // block align, byte rate
   fwrite_int(blockAlign, 2, wavefile.fp);
   // bits per sample, 16 bits
   fwrite_int(wavefile.bits_sample, 2, wavefile.fp);
   // subchunk2 id  data content
   fwrite("data", 4, 1, wavefile.fp);
   // subchunk2 size
   fwrite_int(subchunk2, 4, wavefile.fp);
   return numsa;
}

int fwrite_int(int val, char len, FILE *p){
    unsigned int byte;
    while (len-- > 0) {
        byte = val & 0xFF;
        fwrite(&byte, 1, 1, p);
        val >>= 8;
    }
    return 0;
}

unsigned int fread_int(char len, FILE *p){
    unsigned int byte=0;
    unsigned int val=0;
    unsigned int pc=0;
    while (len-- > 0) {
        fread(&byte, 1, 1, p);
        val = val + (byte << 8*pc);
        pc++;
    }
    return val;
}

void WriteWAVSample(uint16_t sample, WavFileInfo wavefile) {
  fwrite(&sample,2,1,wavefile.fp);
}
