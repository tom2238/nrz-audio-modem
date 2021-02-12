#include "main.h"
#include "parsewav.h"
#include "frame.h"

WavFileInfo wavefile = {WF_SAMPLE_RATE,WF_SAMPLEBITS,WF_CHANNELS,0,0,NULL,0};
GetOptSettings optsettings = {_ABIT_FILE_NO_SET,_ABIT_FILE_NO_SET,DATA_BAUD_RATE, WF_SAMPLE_RATE,0,0,FRAME_DEFAULT_LEN,FRAME_MOD_NRZ};
FILE *InputDataFile;

int main(int argc, char *argv[]) {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    char msg[255];
    int i,j;
    InputDataFile = NULL;

    if(argc == 1){
      Usage(argv[0]);
      return ABIT_ERROR_NOERROR;
    }

    int opt = 0;
      while ((opt = getopt(argc, argv, "ho:b:w:i:MRL:")) != -1){
        switch (opt) {
        case 'h': //Help
          Usage(argv[0]);
          return ABIT_ERROR_NOERROR;
          break;
        case 'o': //Output WAV file
          strncpy(optsettings.filename,optarg,sizeof(optsettings.filename)-1);
          break;
        case 'i': //Input data file
          strncpy(optsettings.inputfile,optarg,sizeof(optsettings.inputfile)-1);
          break;
        case 'b': //Baud rate
          optsettings.baudrate = atoi(optarg);
          if(optsettings.baudrate <= 0) {
             optsettings.baudrate = (int)DATA_BAUD_RATE;
             fprintf(stderr,"Wrong baud rate, settings it to default.\n");
          }
          break;
        case 'w': //WAV file sample rate
          optsettings.wavsamplerate = atoi(optarg);
          if(optsettings.wavsamplerate < 8000) {
             optsettings.wavsamplerate = (int)WF_SAMPLE_RATE;
             fprintf(stderr,"Wrong wav sample rate (minimal 8000 Hz requied), settings it to default.\n");
          }
          break;
        case 'M': //Use Manchester frame modulation
          optsettings.frame_modulation = FRAME_MOD_MAN;
          break;
        case 'R': //RAW output
          optsettings.rawoutput = 1;
          fprintf(stderr,"RAW output\n");
          break;
        case 'L': //Set frame lenght
          optsettings.framelength = atoi(optarg) + HEAD_SIZE + ECC_SIZE + CRC_SIZE;
          break;
        case '?': //Unknown option
          //printf("  Error: %c\n", optopt);
          return 1;
        default:
          Usage(argv[0]);
          return ABIT_ERROR_NOERROR;
        }
      }
      // Output WAV file
      if(strncmp(optsettings.filename,_ABIT_FILE_NO_SET,4) == 0) {
        printf("%s: required argument and option -- '-o <filename>'\n",argv[0]);
        exit(ABIT_ERROR_FILENOSET);
      }
      else if(strncmp(optsettings.filename,_ABIT_FILE_STDOUT,2) == 0) {
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
      // Input data file
      if(strncmp(optsettings.inputfile,_ABIT_FILE_NO_SET,4) == 0) {
        optsettings.zeroframe = 1;
      }
      else if(strncmp(optsettings.inputfile,_ABIT_FILE_STDIN,2) == 0) {
        InputDataFile = stdin;
      }
      else {
        if((InputDataFile = fopen(optsettings.inputfile, "rb")) == NULL){
          strcpy(msg, "Error opening ");
          strcat(msg, optsettings.inputfile);
          perror(msg);
          exit(ABIT_ERROR_FILEOPEN);
        }
      }

    wavefile.sample_rate = optsettings.wavsamplerate;
    wavefile.samples_per_bit = (float)optsettings.wavsamplerate/(float)optsettings.baudrate;

    if(optsettings.frame_modulation == FRAME_MOD_MAN) {
      if(optsettings.framelength > FRAME_LEN_MAX/2) {
        fprintf(stderr,"Frame length %d is too long for Manchester, maximum %d length is used now.\n",optsettings.framelength,FRAME_LEN_MAX/2);
        optsettings.framelength = FRAME_LEN_MAX/2;
      }
    } else {
      if(optsettings.framelength > FRAME_LEN_MAX) {
        fprintf(stderr,"Frame length %d is too long, maximum %d length is used now.\n",optsettings.framelength,FRAME_LEN_MAX);
        optsettings.framelength = FRAME_LEN_MAX;
      }
    }
    if(optsettings.framelength < FRAME_LEN_MIN) {
        fprintf(stderr,"Frame length %d is too low, minimum %d length is used now.\n",optsettings.framelength,FRAME_LEN_MIN);
        optsettings.framelength = FRAME_LEN_MIN;
    }

    fprintf(stderr,"Baud rate: %d\n",optsettings.baudrate);
    fprintf(stderr,"Sample rate: %d\n",optsettings.wavsamplerate);
    fprintf(stderr,"Samples per bit: %f\n",wavefile.samples_per_bit);
    fprintf(stderr,"CRC size: %d bits\n",CRC_SIZE*8);
    fprintf(stderr,"ECC size: %d bits\n",ECC_SIZE*8);
    fprintf(stderr,"Header size: %d bits\n",HEAD_SIZE*8);
    fprintf(stderr,"Frame size: %d bits\n",optsettings.framelength*8);

    float frame_time =  optsettings.framelength * 8 / (float)(optsettings.baudrate);
    fprintf(stderr,"Frame time: %f\n",frame_time);

    float usefull_data_rate = (100*((float)(optsettings.framelength)-(CRC_SIZE+ECC_SIZE+HEAD_SIZE/8)))/((float)(optsettings.framelength));
    fprintf(stderr,"Usefull data rate: %f%%\n",usefull_data_rate);

    if(!optsettings.rawoutput) {
      WriteWAVHeader(frame_time, wavefile);
    }

    uint8_t dataframe_byte;
    uint8_t dataframe_bits[8];
    float sample_bits = 0;
    uint32_t framecount = 0;
    FrameData dataframe;

    while(1) {
      dataframe = NewFrameData(optsettings.framelength,optsettings.frame_modulation);
      if(optsettings.zeroframe) {
        dataframe.value[8] = (framecount >> 24) & 0xFF;
        dataframe.value[9] = (framecount >> 16) & 0xFF;
        dataframe.value[10] = (framecount >> 8) & 0xFF;
        dataframe.value[11] = (framecount >> 0) & 0xFF;
      }
      else {
        if(feof(InputDataFile)) { // If end of file
          if(strncmp(optsettings.filename,_ABIT_FILE_STDOUT,2) != 0) { // Is True, stdout
            if(!optsettings.rawoutput) { // Is not raw output
              fseek(wavefile.fp, 0, SEEK_SET);
              if(optsettings.frame_modulation == FRAME_MOD_NRZ) {
                WriteWAVHeader((float)(framecount)*dataframe.length*8/((float)(optsettings.baudrate)),wavefile);
              } else {
                WriteWAVHeader((float)(framecount)*dataframe.length*8*2/((float)(optsettings.baudrate)),wavefile);
              }
            }
          }
          break;
        }
        for(i=FRAME_START+1;i<dataframe.length-CRC_SIZE;i++) {
          dataframe.value[i] = (uint8_t)(fread_int(1,InputDataFile) & 0xFF);
        }
      }
      CalculateCRC16(&dataframe);
      // Print frame to console, delete in future, debug only
      if(strncmp(optsettings.filename,_ABIT_FILE_STDOUT,2) != 0) { // Is True
        /*printf("Original frame:\n");*/
        PrintFrameData(dataframe);
        /*FrameManchesterEncode(&dataframe,FRAME_START+1);
        printf("Manchester frame:\n");
        PrintFrameData(dataframe);
        printf("Erc: %d \t",FrameManchesterDecode(&dataframe,FRAME_START+1));
        printf("Decoded frame:\n");
        PrintFrameData(dataframe); */
      }
      if(optsettings.frame_modulation == FRAME_MOD_MAN) {
        FrameManchesterEncode(&dataframe,FRAME_START+1); // Make Manchester frame
      } else {
        FrameXOR(&dataframe,0);  // XORing NRZ frame
      }
      fprintf(stderr,"Frame lenght %d\n",dataframe.length);
      for(i=0;i<dataframe.length;i++) { // Full frame include Head + data + CRC + ECC
        dataframe_byte = dataframe.value[i];
        for(j=0;j<8;j++) {
          dataframe_bits[j] = (dataframe_byte >> j) & 0x01;
          while(1) {
            WriteWAVSample((int)((32*32*20*(dataframe_bits[j]*2-1))),wavefile);
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
    if(wavefile.fp != NULL) {
      fclose(wavefile.fp);
    }
    if(InputDataFile != NULL) {
      fclose(InputDataFile);
    }
    return ABIT_ERROR_NOERROR;
}

void Usage(char *p_name) {
  printf("Audio bit encoder\n");
  printf("Usage: %s -o <filename> [-i <filename> -b <rate> -w <rate> -M -R -L <frame length>] | -h\n",p_name);
  printf("  -o <filename> Output WAV file\n");
  printf("  -o -          Write to stdout\n");
  printf("  -R            RAW output\n");
  printf("  -M            Use Manchester coding, default is NRZ\n");
  printf("  -i <filename> Data file to read\n");
  printf("  -i -          Read from stdin\n");
  printf("  -b <rate>     Signal baud rate, default 4800 bit/s\n");
  printf("  -w <rate>     WAV file sample rate, default 24000 Hz\n");
  printf("  -L <frm len>  Set usefull data lenght in bytes, default %d bytes, minimum 8\n",FRAME_DEFAULT_LEN-(HEAD_SIZE+ECC_SIZE+CRC_SIZE));
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
  printf("Run:\n");
  printf("./encoder -o - -b 4800 -w 44100 | sox --ignore-length - -t wav - fir gauss2.4.txt  | play -t wav -\n");
}

void SignalHandler(int number) {
   fprintf(stderr,"\nCaught signal %d ... ", number);
   if(wavefile.fp != NULL) {
     fclose(wavefile.fp);
   }
   if(InputDataFile != NULL) {
     fclose(InputDataFile);
   }
   fprintf(stderr,"abort\n");
   exit(ABIT_ERROR_SIGNAL);
}
