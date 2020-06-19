#include "main.h"
#include "parsewav.h"

GetOptSettings optsettings = {_ABIT_FILE_NO_SET};
WavFileInfo wavefile = {0,0,0,0,0,NULL};

int main(int argc, char *argv[]) {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  char msg[255];

  if(argc == 1){
    Usage(argv[0]);
    return 0;
  }

  int opt = 0;
    while ((opt = getopt(argc, argv, "hi:")) != -1){
      switch (opt) {
      case 'h': //Help
        Usage(argv[0]);
        return 0;
        break;
      case 'i': //Input WAV file
        strncpy(optsettings.filename,optarg,sizeof(optsettings.filename)-1);
        break;
      case '?': //Unknown option
        //printf("  Error: %c\n", optopt);
        return 1;
      default:
        Usage(argv[0]);
        return 0;
      }
    }

    if(strncmp(optsettings.filename,_ABIT_FILE_STDIN,2) == 0) {
      wavefile.fp = stdin;
    }
    else {
      if((wavefile.fp = fopen(optsettings.filename, "rb")) == NULL){
        strcpy(msg, "Error opening ");
        strcat(msg, optsettings.filename);
        perror(msg);
        exit(-2);
      }
    }
    wavefile = ReadWAVHeader(wavefile.fp);
  return 0;
}

void Usage(char *p_name) {
  printf("Audio bit decoder, based on RS41\n");
  printf("Usage: %s (-i filename)| -h\n",p_name);
  printf("  -i <filename> Input WAV file\n");
  printf("  -i -          Read from stdin\n");
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
}

void SignalHandler(int number) {
   printf("\nCaught signal %d ... ", number);
   printf("abort\n");
   exit(-1);
}
