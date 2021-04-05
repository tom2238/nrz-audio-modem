# Audio NRZ/Manchester modem

## Some information
* Decoder is based on zilog80 Vaisala RS41 decoder https://github.com/rs1729/RS/
* Using same header as RS41 for NRZ, for Manchester use extended header Manchester header (similar to DFM)
* File > Encoder > Transmission line + noise > Decoder > File
* Eg. Arduino > RF modulator from STB > Transmission line + noise > RTL-SDR > Decoder > collectd > RRD Graphs
* Eg. RS41 radiosonde > Transmission line + noise > RTL-SDR > nrz-audio-modem
* Manchester from NRZ is coded like: 01->1,10->0 , Maybe it is inverted if my system use little endian byte order. (I'm not sure now.)

### Frame structure

Frame structure for speed 4800 baud/s, 16 user bytes => total 26 bytes (HEAD+DATA+CRC).
* NRZ coding:

```
HEAD = "0000100001101101010100111000100001000100011010010100100000011111"

/--------------*-------------------------*-----------------------\
| HEAD 64 bits | NRZ coded DATA 128 bits | NRZ coded CRC 16 bits |
\--------------*-------------------------*-----------------------/

Total: 208 bits -> 26 bytes

```

* Manchester coding:
```
HEAD = "0101100110011001100110011001100110010101101101101010101010101010"

/--------------*--------------------------------*------------------------------\
| HEAD 64 bits | Manchester coded DATA 256 bits | Manchester coded CRC 32 bits |
\--------------*--------------------------------*------------------------------/

Total: 352 bits -> 44 bytes

```

### Programs help:
* Decoder
```
Audio NRZ/Manchester decoder
Usage: ./decoder -i <filename> [-o <filename> -IRA -b <rate> -M -L <frame length> ]| -h
  -i <filename> Input 8 or 16 bit WAV file
  -i -          Read from stdin
  -o <filename> Output data file
  -o -          Write to stdout
  -b <rate>     Signal baud rate, default 4800
  -L <frm len>  Set usefull data lenght in bytes, default 246 bytes, minimum 8
  -M            Use Manchester coding, default is NRZ
  -I            Inverse signal
  -R            Better bit resolution
  -A            Average decoding
  -P <mode>     Frame printing mode number
                0 - HEX frame output, default
                1 - Decoding from STM32 bluepill test
  -h            Show this help
                Build: 21:11:11 Apr  5 2021, GCC 5.3.0
Run:
sox -t pulseaudio default -t wav - 2>/dev/null | ./decoder -i -
Decoder is under test with STM32 Blue pill board and Si5351 transmitter
```
	
* Encoder
```
Audio NRZ/Manchester encoder
Usage: ./encoder -o <filename> [-i <filename> -b <rate> -w <rate> -M -R -L <frame length>] | -h
  -i <filename> Data file to read
  -i -          Read from stdin
  -o <filename> Output WAV file
  -o -          Write to stdout
  -b <rate>     Signal baud rate, default 4800 bit/s
  -L <frm len>  Set usefull data lenght in bytes, default 246 bytes, minimum 8
  -M            Use Manchester coding, default is NRZ
  -R            RAW output
  -w <rate>     WAV file sample rate, default 24000 Hz
  -h            Show this help
                Build: 13:51:44 Feb 13 2021, GCC 5.3.0
Run:
./encoder -o - -b 4800 -w 44100 | sox --ignore-length - -t wav - fir gauss2.4.txt  | play -t wav -
```	

## TODO
* Implement Manchester code => Finished
* Add Reed Solomon ECC
* Better header detection
* Create some Arduino encoder sketch sending small data packet (like temperature) => Finished https://github.com/tom2238/radiosonde_hacking/tree/main/rs41
* Some bugs and features :-)
