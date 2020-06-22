#include "main.h"
#include "parsewav.h"
#include "readbits.h"

GetOptSettings optsettings = {_ABIT_FILE_NO_SET,0,0,0,0};
WavFileInfo wavefile = {0,0,0,0,0,NULL,0};
RBits readingbits;

int main(int argc, char *argv[]) {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  char msg[255];
  int i;

  if(argc == 1){
    Usage(argv[0]);
    return 0;
  }

  int opt = 0;
    while ((opt = getopt(argc, argv, "hi:IRA")) != -1){
      switch (opt) {
      case 'h': //Help
        Usage(argv[0]);
        return 0;
        break;
      case 'i': //Input WAV file
        strncpy(optsettings.filename,optarg,sizeof(optsettings.filename)-1);
        break;
      case 'I': //Inverse signal
        optsettings.inverse = 1;
        break;
      case 'R': //Better bit resolution
        optsettings.resolution = 1;
        break;
      case 'A': //Average decoding
        optsettings.average = 1;
        break;
      case '?': //Unknown option
        //printf("  Error: %c\n", optopt);
        return 1;
      default:
        Usage(argv[0]);
        return 0;
      }
    }

    if(strncmp(optsettings.filename,_ABIT_FILE_NO_SET,4) == 0) {
      printf("%s: required argument and option -- '-i <filename>'\n",argv[0]);
      exit(-2);
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
    for(i=0;i<LEN_movAvg;i++) {
      readingbits.movAvg[i] = 0;
    }
    readingbits.sample_count = 0;
    readingbits.bufvar = NULL;
    readingbits.mu = 0;
    readingbits.qsum = 0;
    readingbits.xsum = 0;
    readingbits.Nvar = 0;
    readingbits.bitstart = 0;

    wavefile = ReadWAVHeader(wavefile.fp);
    if(wavefile.ret != 0) {
      fprintf(stderr, "Error reading WAV file ... exit!\n");
      exit(wavefile.ret);
    }

    // Reading part
    /*if (optsettings.altdemod)
    {
        Nvar = 32*samples_per_bit;
        bufvar  = (float *)calloc( Nvar+1, sizeof(float)); if (bufvar  == NULL) return -1;
        for (i = 0; i < Nvar; i++) bufvar[i] = 0;
    }*/
    // while (...) ...

  return 0;
}

void Usage(char *p_name) {
  printf("Audio bit decoder, based on RS41\n");
  printf("Usage: %s (-i filename [-IRA])| -h\n",p_name);
  printf("  -i <filename> Input 8 or 16 bit WAV file\n");
  printf("  -i -          Read from stdin\n");
  printf("  -I            Inverse signal\n");
  printf("  -R            Better bit resolution\n");
  printf("  -A            Average decoding\n");
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
}

void SignalHandler(int number) {
   printf("\nCaught signal %d ... ", number);
   printf("abort\n");
   exit(-1);
}
