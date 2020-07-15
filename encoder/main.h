#ifndef AUDIO_ENC_BIT_MAIN_H
#define AUDIO_ENC_BIT_MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <math.h>

// Files
#define _ABIT_FILE_NO_SET "σame"
#define _ABIT_FILE_STDOUT "-"
#define _ABIT_FILE_STDIN  "-"
// Error return code
#define ABIT_ERROR_NOERROR 0
#define ABIT_ERROR_SIGNAL 1
#define ABIT_ERROR_FILENOSET 2
#define ABIT_ERROR_FILEOPEN 3

typedef struct {
  char filename[1024];  // output wav file
  char inputfile[1024]; // input data file
  int baudrate; // Data baud rate
  int wavsamplerate; // WAV file sample rate
  int rawoutput; // RAW sound samples at output
  int zeroframe; // Without input file produce zero frames
}GetOptSettings;

// Print help and usage of application
void Usage(char *p_name);
// Signal handler
void SignalHandler(int number);

#endif // AUDIO_ENC_BIT_MAIN_H
