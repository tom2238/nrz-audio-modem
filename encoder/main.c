#include "main.h"
#include "parsewav.h"
#include "frame.h"

WavFileInfo wavefile = {0,0,0,0,0,NULL,0};
GetOptSettings optsettings = {_ABIT_FILE_NO_SET,DATA_BAUD_RATE};

int main(int argc, char *argv[]) {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    char msg[255];
    int i;

    if(argc == 1){
      Usage(argv[0]);
      return ABIT_ERROR_NOERROR;
    }

    int opt = 0;
      while ((opt = getopt(argc, argv, "ho:b:")) != -1){
        switch (opt) {
        case 'h': //Help
          Usage(argv[0]);
          return ABIT_ERROR_NOERROR;
          break;
        case 'o': //Output WAV file
          strncpy(optsettings.filename,optarg,sizeof(optsettings.filename)-1);
          break;
        case 'b': //Baud rate
          optsettings.baudrate = atoi(optarg);
          if(optsettings.baudrate <= 0) {
             optsettings.baudrate = (int)DATA_BAUD_RATE;
             fprintf(stderr,"Wrong baud rate, setting to default\n");
          }
          break;
        case '?': //Unknown option
          //printf("  Error: %c\n", optopt);
          return 1;
        default:
          Usage(argv[0]);
          return ABIT_ERROR_NOERROR;
        }
      }
      if(strncmp(optsettings.filename,_ABIT_FILE_NO_SET,4) == 0) {
        printf("%s: required argument and option -- '-o <filename>'\n",argv[0]);
        exit(ABIT_ERROR_FILENOSET);
      }
      if(strncmp(optsettings.filename,_ABIT_FILE_STDOUT,2) == 0) {
        wavefile.fp = stdout;
      }
      else {
        if((wavefile.fp = fopen(optsettings.filename, "wb")) == NULL){
          strcpy(msg, "Error opening ");
          strcat(msg, optsettings.filename);
          perror(msg);
          exit(ABIT_ERROR_FILEOPEN);
        }
      }
    printf("Baud Rate: %d\n",optsettings.baudrate);
    fclose(wavefile.fp);
    return ABIT_ERROR_NOERROR;
}

void Usage(char *p_name) {
  printf("Audio bit encoder, based on RS41\n");
  printf("Usage: %s (-o <filename> ) -b <rate>| -h\n",p_name);
  printf("  -o <filename> Output WAV file\n");
  printf("  -o -          Write to stdout\n");
  printf("  -b <rate>     Signal baud rate, default 4800\n");
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
}

void SignalHandler(int number) {
   printf("\nCaught signal %d ... ", number);
   fclose(wavefile.fp);
   printf("abort\n");
   exit(ABIT_ERROR_SIGNAL);
}
