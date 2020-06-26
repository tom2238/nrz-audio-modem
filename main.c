#include "main.h"
#include "parsewav.h"
#include "readbits.h"

GetOptSettings optsettings = {_ABIT_FILE_NO_SET,0,0,0,0};
WavFileInfo wavefile = {0,0,0,0,0,NULL,0};
RBits readingbits;
BufferHead bufferheader = {"0000100001101101010100111000100001000100011010010100100000011111", "x", -1};

int main(int argc, char *argv[]) {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  char msg[255];
  int i;
  int bit = 0;
  int len = 0;
  int byte = 0;
  int byte_count = FRAMESTART;
  int bit_count = 0;
  int header_found = 0;
  int frmlen = FRAME_LEN;
  int ft_len = FRAME_LEN;
  char bitbuf[8];
  //uint8_t xframe[FRAME_LEN] = { 0x10, 0xB6, 0xCA, 0x11, 0x22, 0x96, 0x12, 0xF8},    = xorbyte( frame)
  uint8_t frame[FRAME_LEN] = { 0x86, 0x35, 0xf4, 0x40, 0x93, 0xdf, 0x1a, 0x60}; // = xorbyte(xframe)
  int sumQ = 0, bitQ = 0, Qerror_count = 0;

  if(argc == 1){
    Usage(argv[0]);
    return ABIT_ERROR_NOERROR;
  }

  int opt = 0;
    while ((opt = getopt(argc, argv, "hi:IRA")) != -1){
      switch (opt) {
      case 'h': //Help
        Usage(argv[0]);
        return ABIT_ERROR_NOERROR;
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
        return ABIT_ERROR_NOERROR;
      }
    }

    if(strncmp(optsettings.filename,_ABIT_FILE_NO_SET,4) == 0) {
      printf("%s: required argument and option -- '-i <filename>'\n",argv[0]);
      exit(ABIT_ERROR_FILENOSET);
    }
    if(strncmp(optsettings.filename,_ABIT_FILE_STDIN,2) == 0) {
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

    wavefile = ReadWAVHeader(wavefile.fp);
    if(wavefile.ret != 0) {
      fprintf(stderr, "Error reading WAV file ... exit!\n");
      exit(wavefile.ret);
    }

    // Reading part
    if (optsettings.altdemod) {
        readingbits.Nvar = 32*wavefile.samples_per_bit;
        readingbits.bufvar = (float *)calloc( readingbits.Nvar+1, sizeof(float));
        if (readingbits.bufvar  == NULL) {
          return ABIT_ERROR_CALLOC;
        }
        for (i = 0; i < readingbits.Nvar; i++) {
          readingbits.bufvar[i] = 0;
        }
    }

    while (!ReadBitsFSK(wavefile,optsettings,&readingbits,&bit,&len)) {
      if (len == 0) { // reset_frame();
        if (byte_count > pos_AUX) {
          printf("Print frame len==0\n");
          bit_count = 0;
          byte_count = FRAMESTART;
          header_found = 0;
        }

        //IncBufPos();
        //buf[bufpos] = 'x';
        continue;   // ...
      }

      for (i = 0; i < len; i++) {
        IncBufPos();
        bufferheader.buf[bufferheader.bufpos] = 0x30 + bit;  // Ascii

        if (!header_found) {
          if (Compare() >= HEADLEN) {
            header_found = 1;
            printf("Header found\n");
          }
        }
        else {
          bitbuf[bit_count] = bit;
          bit_count++;

          if (bit_count == 8) {
            bit_count = 0;
            byte = Bits2Byte(bitbuf);
            //xframe[byte_count] = byte;
            //frame[byte_count] = byte ^ mask[byte_count % MASK_LEN];
            frame[byte_count] = byte;
            byte_count++;

            if (byte_count == frmlen) {
              byte_count = FRAMESTART;
              header_found = 0;
              printf("Print frame after count==frmlen\n");
            }
          }
        }
      }
      if (header_found && optsettings.altdemod) {
        readingbits.bitstart = 1;
        sumQ = 0;
        Qerror_count = 0;
        ft_len = frmlen;

        while ( byte_count < frmlen ) {
          bitQ = ReadRawbit(wavefile,optsettings,&readingbits,&bit); // return: zeroX/bit (oder alternativ Varianz/bit)
          if ( bitQ == EOF) {
            break;
          }
          sumQ += bitQ; // zeroX/byte
          bitbuf[bit_count] = bit;
          bit_count++;
          if (bit_count == 8) {
            bit_count = 0;
            byte = Bits2Byte(bitbuf);
            //xframe[byte_count] = byte;
            //frame[byte_count] = byte ^ mask[byte_count % MASK_LEN];
            frame[byte_count] = byte;

            readingbits.mu = readingbits.xsum/(float)readingbits.Nvar;
            readingbits.bvar[byte_count] = readingbits.qsum/(float)readingbits.Nvar - readingbits.mu*readingbits.mu;

            if (byte_count > NDATA_LEN) {  // Errors only from minimal framelen counts
              //ratioQ = sumQ/samples_per_bit; // approx: with noise zeroX / byte unfortunately not linear in sample_rate
              //if (ratioQ > 0.7) {            // sr=48k: 0.7, Threshold, from when probably noise bit
              if (readingbits.bvar[byte_count]*2 > readingbits.bvar[byte_count-300]*3) { // Var(frame)/Var(noise) ca. 1:2
                Qerror_count += 1;
              }
            }
            sumQ = 0; // Fenster fuer zeroXcount: 8 bit
            byte_count++;
          }
          ft_len = byte_count;
          Qerror_count += 1;
        }
        header_found = 0;
        printf("Print frame");
        byte_count = FRAMESTART;
      }
    }
    if (optsettings.altdemod) {
      if (readingbits.bufvar)  {
        free(readingbits.bufvar);
        readingbits.bufvar = NULL;
      }
    }

  return ABIT_ERROR_NOERROR;
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
   if (optsettings.altdemod) {
     if (readingbits.bufvar)  {
       free(readingbits.bufvar);
       readingbits.bufvar = NULL;
     }
   }
   printf("abort\n");
   exit(ABIT_ERROR_SIGNAL);
}

void IncBufPos() {
  bufferheader.bufpos = (bufferheader.bufpos+1) % HEADLEN;
}

int Bits2Byte(char bits[]) {
    int i, byteval=0, d=1;
    for (i = 0; i < 8; i++) {     // little endian
    /* for (i = 7; i >= 0; i--) { // big endian */
        if      (bits[i] == 1)  {
            byteval += d;
        }
        else if (bits[i] == 0)  {
            byteval += 0;
        }
        else {
            return 0x100;
        }
        d <<= 1;
    }
    return byteval;
}

int Compare() {
    int i = 0;
    int j = bufferheader.bufpos;

    while (i < HEADLEN) {
      if (j < 0) {
        j = HEADLEN-1;
      }
      if (bufferheader.buf[j] != bufferheader.header[HEADOFS+HEADLEN-1-i]) {
        break;
      }
      j--;
      i++;
    }

    return i;
}
