#include "main.h"
#include "readbits.h"
#include "parsewav.h"

int ReadBitsFSK(WavFileInfo wave, GetOptSettings setup, RBits *databits ,int *bit, int *len) {
    static int sample = 0;
    int n = 0, y0;
    float l, x1;
    static float x0;

    do{
        y0 = sample;
        sample = ReadSignedSample(wave, setup, databits);
        if (sample == EOF_INT) {
          return EOF;
        }
        //sample_count++; // in ReadSignedSample()
        databits->par_alt = databits->par;
        databits->par =  (sample >= 0) ? 1 : -1;  // 8bit: 0..127,128..255 (-128..-1,0..127)
        n++;
    } while (((databits->par)*(databits->par_alt)) > 0);

    if (!setup.resolution) {
        l = (float)n / wave.samples_per_bit;
    }
    else {                                 // more precise bit length measurement
        x1 = sample/(float)(sample-y0);    // helps with low sample rate
        l = (n+x0-x1) / wave.samples_per_bit;   // mostly more frames (not always)
        x0 = x1;
    }

    *len = (int)(l+0.5);

    if (!setup.inverse){
        *bit = (1+databits->par_alt)/2;  // top 1, bottom -1
    }
    else {
        *bit = (1-databits->par_alt)/2;  // sdr # <rev1381?, inverse: bottom 1, top -1
    }

    /* Y-offset ? */

    return 0;
}

int ReadSignedSample(WavFileInfo wave, GetOptSettings setup, RBits *databits) {  // int = i32_t
    int byte, i, sample=0, s=0;     // EOF -> 0x1000000
    float x=0, x0=0;

    for (i = 0; i < wave.channels; i++) { // i = 0: links bzw. mono
        byte = fgetc(wave.fp);
        if (byte == EOF) {
          return EOF_INT;
        }
        if (i == wave.wav_channel) {
          sample = byte;
        }
        if (wave.bits_sample == 16) {
          byte = fgetc(wave.fp);
          if (byte == EOF) {
            return EOF_INT;
          }
          if (i == wave.wav_channel) {
            sample +=  byte << 8;
          }
        }
    }

    if (wave.bits_sample ==  8) {
      s = sample-128;   // 8bit: 00..FF, centerpoint 0x80=128
    }
    if (wave.bits_sample == 16) {
      s = (short)sample;
    }
    if (setup.altdemod)
    {
        x = s/128.0;
        if (wave.bits_sample == 16) {
            x /= 256.0;
        }
        databits->bufvar[databits->sample_count % databits->Nvar] = x;
        x0 = databits->bufvar[(databits->sample_count+1) % databits->Nvar];
        databits->xsum = databits->xsum - x0 + x;
        databits->qsum = databits->qsum - x0*x0 + x*x;
    }

    if (setup.average) {
        databits->movAvg[databits->sample_count % LEN_movAvg] = s;
        s = 0;
        for (i = 0; i < LEN_movAvg; i++) {
          s += databits->movAvg[i];
        }
        s = (s+0.5) / LEN_movAvg;
    }

    databits->sample_count++;

    return s;
}

int ReadRawbit(WavFileInfo wave, GetOptSettings setup, RBits *databits,int *bit) {
    int sample = 0;
    int sum = 0;
    int sample0 = 0;
    int pars = 0;

    if (databits->bitstart) {
        databits->scount = 1;    // (sample_count overflow/wrap-around)
        databits->bitgrenze = 0; // d.h. bitgrenze = sample_count-1 (?)
        databits->bitstart = 0;
    }
    databits->bitgrenze += wave.samples_per_bit;

    do {
        sample = ReadSignedSample(wave, setup, databits);
        if (sample == EOF_INT) {
          return EOF;
        }
        //sample_count++; // in ReadSignedSample()
        //par =  (sample >= 0) ? 1 : -1;    // 8bit: 0..127,128..255 (-128..-1,0..127)
        sum += sample;

        if (sample * sample0 < 0) {
          pars++;   // wenn sample[0..n-1]=0 ...
        }
        sample0 = sample;

        databits->scount++;
    } while (databits->scount < databits->bitgrenze);  // n < samples_per_bit

    if (sum >= 0) {
      *bit = 1;
    }
    else {
      *bit = 0;
    }
    if (setup.inverse) {
      *bit ^= 1;
    }
    return pars;
}
