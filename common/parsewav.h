#ifndef AUDIO_BIT_PARSEWAV_H
#define AUDIO_BIT_PARSEWAV_H

#define ABIT_ERROR_WAVREADING 4
#define ABIT_ERROR_WAVBITS 5

typedef struct {
  int sample_rate; // WAV sample rate eg. 44100 Hz
  int bits_sample; // 8 or 16 bit
  int channels; // mono or stereo
  float samples_per_bit;
  int ret;  // exit status
  FILE *fp; // input file
  int wav_channel; // 0 left
}WavFileInfo;

// Read WAVe file header
WavFileInfo ReadWAVHeader(FILE *fp, int baudrate);
// Find some chars in string at position
int findstr(char *buff, char *str, int pos);

#endif // AUDIO_BIT_PARSEWAV_H
