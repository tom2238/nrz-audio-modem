#ifndef AUDIO_BIT_PARSEWAV_H
#define AUDIO_BIT_PARSEWAV_H

typedef struct {
  int sample_rate;
  int bits_sample;
  int channels;
  float samples_per_bit;
  int ret;
  FILE *fp;
}WavFileInfo;

// Read WAVe file header
WavFileInfo ReadWAVHeader(FILE *fp);
// Find some chars in string at position
int findstr(char *buff, char *str, int pos);

#endif // AUDIO_BIT_PARSEWAV_H
