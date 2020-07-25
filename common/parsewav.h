#ifndef AUDIO_BIT_PARSEWAV_H
#define AUDIO_BIT_PARSEWAV_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define ABIT_ERROR_WAVREADING 4
#define ABIT_ERROR_WAVBITS 5
//Math
#define   WF_TPI   6.283185307
//Device Parameters
#define   WF_SAMPLE_RATE  24000 // Default sample rate 24khz
#define   WF_CHANNELS    1     // mono
#define   WF_SAMPLEBITS  16    // 16bits
#define   WF_BUFFER_DIV WF_SAMPLEBITS/8 //buffer divider

typedef struct {
  int sample_rate; // WAV sample rate eg. 44100 Hz
  int bits_sample; // 8 or 16 bit
  int channels; // mono or stereo
  float samples_per_bit;
  int ret;  // exit status
  FILE *fp; // input/output file
  int wav_channel; // 0 left
}WavFileInfo;

// Read WAVe file header
WavFileInfo ReadWAVHeader(FILE *fp, int baudrate);
// Find some chars in string at position
int findstr(char *buff, char *str, int pos);
//Write wav file header
unsigned int WriteWAVHeader(double tdur, WavFileInfo wavefile);
//Write integer, little endian
int fwrite_int(int val, char len, FILE *p);
//Read integer, little endian
unsigned int fread_int(char len, FILE *p);
//Write one wav audio sample
void WriteWAVSample(uint16_t sample, WavFileInfo wavefile);

#endif // AUDIO_BIT_PARSEWAV_H
