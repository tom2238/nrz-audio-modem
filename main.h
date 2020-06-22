#ifndef AUDIO_BIT_MAIN_H
#define AUDIO_BIT_MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <signal.h>

// Files
#define _ABIT_FILE_NO_SET "σame"
#define _ABIT_FILE_STDOUT "-"
#define _ABIT_FILE_STDIN  "-"
// Data baud rate
#define DATA_BAUD_RATE 2400
// End of integer
#define EOF_INT  0x1000000
//
#define NDATA_LEN 320                    // std framelen 320
#define XDATA_LEN 198
#define FRAME_LEN (NDATA_LEN+XDATA_LEN)  // max framelen 518

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

#endif // AUDIO_BIT_MAIN_H
