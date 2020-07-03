#ifndef AUDIO_ENC_BIT_MAIN_H
#define AUDIO_ENC_BIT_MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

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
  int baudrate; // Data baud rate
}GetOptSettings;

// Print help and usage of application
void Usage(char *p_name);
// Signal handler
void SignalHandler(int number);

#endif // AUDIO_ENC_BIT_MAIN_H
