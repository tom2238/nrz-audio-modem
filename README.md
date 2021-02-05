# Audio NRZ/Manchester modem

## Some information
* Decoder is based on zilog80 Vaisala RS41 decoder https://github.com/rs1729/RS/
* Using same header as RS41 for NRZ, for Manchester use DFM header
* File > Encoder > Transmission line + noise > Decoder > File
* Eg. Arduino > RF modulator from STB > Transmission line + noise > RTL-SDR > Decoder > collectd > RRD Graphs

### Programs help:
* Decoder
```
	Audio bit decoder, based on RS41
	Usage: ./decoder -i <filename> [-o <filename> -IRAD -b <rate> -L <frame length> ]| -h
	  -i <filename> Input 8 or 16 bit WAV file
	  -i -          Read from stdin
	  -o <filename> Output data file
	  -o -          Write to stdout
	  -b <rate>     Signal baud rate, default 4800
	  -L <frm len>  Set frame lenght in bytes, including head + data + ecc + crc, default 256 bytes
	  -I            Inverse signal
	  -R            Better bit resolution
	  -A            Average decoding
	  -D            Alternative demodulation technic
	  -h            Show this help
	                Build: 19:38:55 Feb  5 2021, GCC 5.3.0
	Run:
	sox -t pulseaudio default -t wav - 2>/dev/null | ./decoder -i -
```
	
* Encoder
```
	Audio bit encoder, based on RS41
	Usage: ./encoder -o <filename> [-i <filename> -b <rate> -w <rate> -R -L <frame length>] | -h
	  -o <filename> Output WAV file
	  -o -          Write to stdout
	  -R            RAW output
	  -i <filename> Data file to read
	  -i -          Read from stdin
	  -b <rate>     Signal baud rate, default 4800 bit/s
	  -w <rate>     WAV file sample rate, default 24000 Hz
	  -L <frm len>  Set frame lenght in bytes, including head + data + ecc + crc, default 256 bytes
	  -h            Show this help
	                Build: 19:38:55 Feb  5 2021, GCC 5.3.0
	Run:
	./encoder -o - -b 4800 -w 44100 | sox --ignore-length - -t wav - fir gauss2.4.txt  | play -t wav -
```	

## TODO
* Implement Manchester code
* Add Reed Solomon ECC
* Better header detection
* Create some Arduino encoder sketch sending small data packet (like temperature)
* Some bugs and features :-)
