#include "main.h"
#include "parsewav.h"
#include "readbits.h"
#include "frame.h"

GetOptSettings optsettings = {_ABIT_FILE_NO_SET,_ABIT_FILE_NO_SET,DATA_BAUD_RATE,0,0,0,0,FRAME_DEFAULT_LEN,FRAME_MOD_NRZ,0,0};
WavFileInfo wavefile = {0,0,0,0,0,NULL,0};
RBits readingbits;
FrameHead bufferheader;
FILE *OutputDataFile;

// Debug printf
//#define _PRG_DEBUG

int main(int argc, char *argv[]) {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  char msg[255];  
  OutputDataFile = NULL;

  if(argc == 1){
    Usage(argv[0]);
    return ABIT_ERROR_NOERROR;
  }

  int opt = 0;
    while ((opt = getopt(argc, argv, "hi:o:b:IMRAL:P:F:")) != -1){
      switch (opt) {
      case 'h': //Help
        Usage(argv[0]);
        return ABIT_ERROR_NOERROR;
        break;
      case 'i': //Input WAV file
        strncpy(optsettings.filename,optarg,sizeof(optsettings.filename)-1);
        break;
      case 'o': //Output data file
        strncpy(optsettings.outputfile,optarg,sizeof(optsettings.outputfile)-1);
        break;
      case 'b': //Baud rate
        optsettings.baudrate = atoi(optarg);
        if(optsettings.baudrate <= 0) {
           optsettings.baudrate = (int)DATA_BAUD_RATE;
           fprintf(stderr,"Wrong baud rate, setting to default\n");
        }
        break;
      case 'I': //Inverse signal
        optsettings.inverse = 1;
        break;
      case 'M': //Use Manchester frame modulation
        optsettings.frame_modulation = FRAME_MOD_MAN;
        break;
      case 'R': //Better bit resolution
        optsettings.resolution = 1;
        break;
      case 'A': //Average decoding
        optsettings.average = 1;
        break;
      case 'L': //Set frame lenght
        optsettings.framelength = atoi(optarg) + HEAD_SIZE + ECC_SIZE + CRC_SIZE;
        break;
      case 'P': //Printing mode function (hex, stm32 ...)
        optsettings.printmode = atoi(optarg);
        break;
      case 'F': //Reed-Solomon coding level
        optsettings.ecc_code = atoi(optarg);
        break;
      case '?': //Unknown option
        //printf("  Error: %c\n", optopt);
        return 1;
      default:
        Usage(argv[0]);
        return ABIT_ERROR_NOERROR;
      }
    }
    // Input WAV file
    if(strncmp(optsettings.filename,_ABIT_FILE_NO_SET,4) == 0) {
      printf("%s: required argument and option -- '-i <filename>'\n",argv[0]);
      exit(ABIT_ERROR_FILENOSET);
    }
    else if(strncmp(optsettings.filename,_ABIT_FILE_STDIN,2) == 0) {
      wavefile.fp = stdin;
    }
    else {
      if((wavefile.fp = fopen(optsettings.filename, "rb")) == NULL){
        strcpy(msg, "Error opening ");
        strcat(msg, optsettings.filename);
        perror(msg);
        exit(ABIT_ERROR_FILEOPEN);
      }
    }
    // Output data file
    if(strncmp(optsettings.outputfile,_ABIT_FILE_NO_SET,4) == 0) {
      optsettings.printframe = 1;
    }
    else if(strncmp(optsettings.outputfile,_ABIT_FILE_STDOUT,2) == 0) {
      OutputDataFile = stdout;
    }
    else {
      if((OutputDataFile = fopen(optsettings.outputfile, "wb")) == NULL){
        strcpy(msg, "Error opening ");
        strcat(msg, optsettings.outputfile);
        perror(msg);
        exit(ABIT_ERROR_FILEOPEN);
      }
    }

    // Set Reed-Solomon parity size
    switch (optsettings.ecc_code) {
      case 6: // Level 6 = (255,207)
        optsettings.ecc_code = 48;
        break;
      case 5: // Level 5 = (255,215)
        optsettings.ecc_code = 40;
        break;
      case 4: // Level 4 = (255,223)
        optsettings.ecc_code = 32;
        break;
      case 3: // Level 3 = (255,231)
        optsettings.ecc_code = 24;
        break;
      case 2: // Level 2 = (255,239)
        optsettings.ecc_code = 16;
        break;
      case 1: // Level 1 = (255,247)
        optsettings.ecc_code = 8;
        break;
      default: // Level 0 = uncoded
        optsettings.ecc_code = 0;
    }
    // Add ECC into frame length
    optsettings.framelength += optsettings.ecc_code;
    // Check ECC max size
    if((Frame_CheckRSLimit(optsettings.framelength-HEAD_SIZE-optsettings.ecc_code,optsettings.ecc_code)) && (optsettings.ecc_code!=0)) {
        fprintf(stderr,"Error: Frame length %d is too long for Reed-Solomon coding. Please limit in: DATA(%d) + CRC(%d) + PARITY(%d) <= 255 bytes.\n",optsettings.framelength,optsettings.framelength-HEAD_SIZE-CRC_SIZE-optsettings.ecc_code,CRC_SIZE,optsettings.ecc_code);
        if(wavefile.fp != NULL) {
          fclose(wavefile.fp);
        }
        if(OutputDataFile != NULL) {
          fclose(OutputDataFile);
        }
        exit(ABIT_ERROR_FRAME_LENGTH);
    }
    // Handle Reed-Solomon en/decoder init
    if(optsettings.ecc_code!=0) {
      if(Frame_RSInit(optsettings.framelength-optsettings.ecc_code-HEAD_SIZE,optsettings.framelength-optsettings.ecc_code-HEAD_SIZE,optsettings.ecc_code)) {
        fprintf(stderr,"Error: Initializing Reed-Solomon en/decoder.\n");
        if(wavefile.fp != NULL) {
          fclose(wavefile.fp);
        }
        if(OutputDataFile != NULL) {
          fclose(OutputDataFile);
        }
        exit(ABIT_ERROR_FRAME_LENGTH);
      }
    }
    // Check frame length
    if(optsettings.frame_modulation == FRAME_MOD_MAN) {
      if(optsettings.framelength > FRAME_LEN_MAX/2) {
          fprintf(stderr,"Error: Frame length %d is too long for Manchester. Maximum frame length: %d bytes.\n",optsettings.framelength,FRAME_LEN_MAX/2);
          if(wavefile.fp != NULL) {
            fclose(wavefile.fp);
          }
          if(OutputDataFile != NULL) {
            fclose(OutputDataFile);
          }
          exit(ABIT_ERROR_FRAME_LENGTH);
      }
    } else {
      if(optsettings.framelength > FRAME_LEN_MAX) {
          fprintf(stderr,"Error: Frame length %d is too long. Maximum frame length: %d bytes.\n",optsettings.framelength,FRAME_LEN_MAX);
          if(wavefile.fp != NULL) {
            fclose(wavefile.fp);
          }
          if(OutputDataFile != NULL) {
            fclose(OutputDataFile);
          }
          exit(ABIT_ERROR_FRAME_LENGTH);
      }
    }
    if(optsettings.framelength < FRAME_LEN_MIN) {
        fprintf(stderr,"Error: Frame length %d is too short. Minimum frame length is %d bytes.\n",optsettings.framelength,FRAME_LEN_MIN);
        if(wavefile.fp != NULL) {
          fclose(wavefile.fp);
        }
        if(OutputDataFile != NULL) {
          fclose(OutputDataFile);
        }
        exit(ABIT_ERROR_FRAME_LENGTH);
    }
    // Reserve space for Manchester code in frame
    if(optsettings.frame_modulation == FRAME_MOD_MAN) {
        optsettings.framelength = (optsettings.framelength*2)-HEAD_SIZE;
    }

    // Create new empty frame
    FrameData frame = NewFrameData(optsettings.framelength,optsettings.frame_modulation);
    bufferheader = NewFrameHead(optsettings.frame_modulation);

    int i;
    int bit = 0;
    int len = 0;
    int byte = 0;
    int byte_count = FRAME_START;
    int bit_count = 0;
    int header_found = 0;
    int frmlen = optsettings.framelength;//frame.length;
    //int ft_len = frame.length;
    char bitbuf[8];   

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
    readingbits.par = 1;
    readingbits.par_alt = 1;
    readingbits.bitgrenze = 0;
    readingbits.scount = 0;

    wavefile = ReadWAVHeader(wavefile.fp, optsettings.baudrate);
    if(wavefile.ret != 0) {
      fprintf(stderr, "Error reading WAV file ... exit!\n");
      exit(wavefile.ret);
    }

    while (!ReadBitsFSK(wavefile,optsettings,&readingbits,&bit,&len)) {
      for (i = 0; i < len; i++) {
        IncHeadPos(&bufferheader);
        bufferheader.value[bufferheader.position] = 0x30 + bit;  // Ascii

        if (!header_found) {
          if (FrameHeadCompare(bufferheader) >= HEAD_LEN) {
            header_found = 1;
            if(optsettings.printframe) {
              #ifdef _PRG_DEBUG
                printf("Header found\n");
              #endif
            }
          }
        } else {
            bitbuf[bit_count] = bit;
            bit_count++;

            if (bit_count == 8) {
                bit_count = 0;
                byte = Bits2Byte(bitbuf);
                frame.value[byte_count] = byte;
                byte_count++;

                if (byte_count == frmlen) {
                    byte_count = FRAME_START;
                    header_found = 0;

                    if(optsettings.frame_modulation == FRAME_MOD_MAN) {
                        FrameManchesterDecode(&frame,FRAME_START+1); // Decode Manchester frame
                    } else {
                        FrameXOR(&frame,FRAME_START); // XORing NRZ frame
                    }
                    // Reed-Solomon error correction
                    if(optsettings.ecc_code != 0) {
                        Frame_RSDecode(&frame);
                    }
                    // Printing frame content to console
                    if(optsettings.printframe) {
#ifdef _PRG_DEBUG
                        printf("Print frame after count==frmlen\n");
                        PrintFrameData(frame,optsettings.ecc_code);
#else
                        // Select printing mode
                        switch (optsettings.printmode) {
                        case 3: // Print packet from Vaisala RS41 sounding sonde
                            PrintFrame_RS41Sounding(frame,optsettings.ecc_code);
                            break;
                        case 2: // Print packet from Vaisala RS41 GPS
                            PrintFrame_RS41GPS(frame,optsettings.ecc_code);
                            break;
                        case 1: // Print packet from STM32 blue pill test
                            PrintFrame_STM32(frame,optsettings.ecc_code);
                            break;
                        case 0: // Zero or default is hex output
                        default:
                            PrintFrameData(frame,optsettings.ecc_code);
                        }
#endif
                    } else {
                        // Write frame
                        Frame_WriteToFile(frame,OutputDataFile,optsettings.ecc_code);
                    }
                    frame.length = optsettings.framelength;
                }
            }
        }
      }
    }
    if(wavefile.fp != NULL) {
      fclose(wavefile.fp);
    }
    if(OutputDataFile != NULL) {
      fclose(OutputDataFile);
    }
  return ABIT_ERROR_NOERROR;
}

void Usage(char *p_name) {
  printf("Audio NRZ/Manchester decoder\n");
  printf("Usage: %s -i <filename> [-o <filename> -IRA -b <rate> -M -L <frame length> -F <RS level> ] | -h\n",p_name);
  printf("  -i <filename> Input 8 or 16 bit WAV file\n");
  printf("  -i -          Read from stdin\n");
  printf("  -o <filename> Output data file\n");
  printf("  -o -          Write to stdout\n");
  printf("  -b <rate>     Signal baud rate, default 4800\n");
  printf("  -L <frm len>  Set usefull data lenght in bytes, default %d bytes, minimum 8\n",FRAME_DEFAULT_LEN-(HEAD_SIZE+optsettings.ecc_code+CRC_SIZE));
  printf("  -F <RS level> Check Reed-Solomon parity bytes: 0 = uncoded (default), 1 = (255,247), 2 = (255,239)\n");
  printf("                3 = (255,231), 4 = (255,223), 5 = (255,215), 6 = (255,207)\n");
  printf("  -M            Use Manchester coding, default is NRZ\n");
  printf("  -I            Inverse signal\n");  
  printf("  -R            Better bit resolution\n");
  printf("  -A            Average decoding\n");
  printf("  -P <mode>     Frame printing mode number\n");
  printf("                0 - HEX frame output, default\n");
  printf("                1 - Decoding from STM32 bluepill test\n");
  printf("                2 - Decoding from Vaisala RS41\n");
  printf("                3 - Decoding from Vaisala RS41 sounding sonde\n");
  printf("  -h            Show this help\n");
  printf("                Build: %s %s, GCC %s\n", __TIME__, __DATE__, __VERSION__);
  printf("Run:\n");
  printf("sox -t pulseaudio default -t wav - 2>/dev/null | ./decoder -i -\n");
  printf("Decoder is under test with STM32 Blue pill board and Si5351 transmitter or Vaisala RS41 radiosonde :-)\n");
}

void SignalHandler(int number) {
   fprintf(stderr,"\nCaught signal %d ... ", number);
   if(wavefile.fp != NULL) {
     fclose(wavefile.fp);
   }
   if(OutputDataFile != NULL) {
     fclose(OutputDataFile);
   }
   fprintf(stderr,"abort\n");
   exit(ABIT_ERROR_SIGNAL);
}
