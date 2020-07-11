#include "main.h"
#include "parsewav.h"
#include "frame.h"

WavFileInfo wavefile = {WF_SAMPLE_RATE,WF_SAMPLEBITS,WF_CHANNELS,0,0,NULL,0};
GetOptSettings optsettings = {_ABIT_FILE_NO_SET,DATA_BAUD_RATE, WF_SAMPLE_RATE};

int main(int argc, char *argv[]) {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    char msg[255];
    int i,j;

    if(argc == 1){
      Usage(argv[0]);
      return ABIT_ERROR_NOERROR;
    }

    int opt = 0;
      while ((opt = getopt(argc, argv, "ho:b:w:")) != -1){
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
        case 'w': //WAV file sample rate
          optsettings.wavsamplerate = atoi(optarg);
          if(optsettings.wavsamplerate < 8000) {
             optsettings.wavsamplerate = (int)WF_SAMPLE_RATE;
             fprintf(stderr,"Wrong wav sample rate (min 8000), setting to default\n");
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

    wavefile.sample_rate = optsettings.wavsamplerate;
    wavefile.samples_per_bit = (float)optsettings.wavsamplerate/(float)optsettings.baudrate;



    fprintf(stderr,"Baud Rate: %d\n",optsettings.baudrate);
    fprintf(stderr,"Sample rate: %d\n",optsettings.wavsamplerate);
    fprintf(stderr,"Samples per bit: %f\n",wavefile.samples_per_bit);

    float frame_time = FRAME_LEN * 8 / (float)(optsettings.baudrate);
    fprintf(stderr,"Frame time: %f\n",frame_time);

    WriteWAVHeader(frame_time, wavefile);

    uint8_t dataframe_byte;
    uint8_t dataframe_bits[8];

    float sample_bits = 0;
    uint32_t framecount = 0;
    FrameData dataframe;
    while(1) {
      dataframe = NewFrameData();
      dataframe.value[8] = (framecount >> 24) & 0xFF;
      dataframe.value[9] = (framecount >> 16) & 0xFF;
      dataframe.value[10] = (framecount >> 8) & 0xFF;
      dataframe.value[11] = (framecount >> 0) & 0xFF;
      FrameXOR(&dataframe,0);
      for(i=0;i<dataframe.length;i++) {
        dataframe_byte = dataframe.value[i];
        for(j=0;j<8;j++) {
          dataframe_bits[j] = (dataframe_byte >> j) & 0x01;
          while(1) {
            WriteWAVSample((int)((32*32*10*(dataframe_bits[j]*2-1))),wavefile);
            sample_bits += 1;
            if(sample_bits > wavefile.samples_per_bit) {
              sample_bits -= wavefile.samples_per_bit;
              break;
            }
          }
        }
      }
      framecount++;
    }

    fclose(wavefile.fp);
    return ABIT_ERROR_NOERROR;
}

void Usage(char *p_name) {
  printf("Audio bit encoder, based on RS41\n");
  printf("Usage: %s (-o <filename> ) -b <rate> -w <rate> | -h\n",p_name);
  printf("  -o <filename> Output WAV file\n");
  printf("  -o -          Write to stdout\n");
  printf("  -b <rate>     Signal baud rate, default 4800 b/s\n");
  printf("  -w <rate>     WAV file sample rate, default 24000 Hz\n");
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
  printf("Run:\n");
  printf("./encoder -o - -b 4800 -w 44100 | sox --ignore-length - -t wav - fir gauss2.4.txt  | play -t wav -\n");
}

void SignalHandler(int number) {
   fprintf(stderr,"\nCaught signal %d ... ", number);
   fclose(wavefile.fp);
   fprintf(stderr,"abort\n");
   exit(ABIT_ERROR_SIGNAL);
}
