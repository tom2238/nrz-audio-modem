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

typedef struct {
  char filename[1024];  // Input wav file
}GetOptSettings;

// Print help and usage of application
void Usage(char *p_name);
// Signal handler
void SignalHandler(int number);

#endif // AUDIO_BIT_MAIN_H
