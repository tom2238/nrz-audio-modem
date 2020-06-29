#ifndef AUDIO_BIT_MAIN_H
#define AUDIO_BIT_MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>

// Files
#define _ABIT_FILE_NO_SET "σame"
#define _ABIT_FILE_STDOUT "-"
#define _ABIT_FILE_STDIN  "-"
// Data baud rate
#define DATA_BAUD_RATE 2400
// End of integer
#define EOF_INT  0x1000000
#define MASK_LEN 64
// Error return code
#define ABIT_ERROR_NOERROR 0
#define ABIT_ERROR_SIGNAL 1
#define ABIT_ERROR_FILENOSET 2
#define ABIT_ERROR_FILEOPEN 3
#define ABIT_ERROR_WAVREADING 4
#define ABIT_ERROR_WAVBITS 5
#define ABIT_ERROR_CALLOC 6

typedef struct {
  char filename[1024];  // input wav file
  int inverse; // inverted signal
  int resolution; // more accurate bit measurement
  int altdemod; // alternative demod technic
  int average; // moving average
}GetOptSettings;

// Print help and usage of application
void Usage(char *p_name);
// Signal handler
void SignalHandler(int number);
//
int Bits2Byte(char bits[]);

#endif // AUDIO_BIT_MAIN_H
