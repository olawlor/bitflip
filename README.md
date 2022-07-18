# Bitflip Detection Tool

This program watches for changes to a configurable block of allocated memory. This can be used to detect cosmic ray induced bit flips, at least on susceptible hardware. 

At startup the program writes a 257-byte repeating pattern to memory, then slowly scans through watching for changes.  It's designed to keep running for months or years, so it only checks 1 megabyte at a time between sleep statements, and uses less than 1% of one CPU core while running. 

## Build and Run

To build on a UNIX type system (with usleep):
```
make
```

To continually check a block of 1 gig = 1024 megs of RAM:
```
./bitflip 1024 &
```

Any errors are reported to /tmp/bitflip.log and stderr. 

On a 64-bit system, this has been tested and will work with more than 4 gig buffers.

On a low-end system like a Raspberry Pi, you can adjust the usleep at the end of bitflip.cpp to reduce the CPU usage. `usleep(1000*1000);` uses 0.2% of one core of a Raspberry Pi 4. 


## Detected Errors
This program has successfully detected exactly one bit error so far, on a server in 40GB buffer:
RAM MISMATCH DETECTED: Index 15923718329 should contain e6 actually had e2 (flip 04)
This was a bitflip from 1 to 0. It took *months* for this to happen while watching 40GB of ram though, and has not repeated since. 

I have been watching 2GB of a Raspberry Pi 4's RAM for several years with no detected bit flips.

If the program detects any RAM mismatches for you, please mail the output and your brief hardware info to me at lawlor@alaska.edu because I'm curious about their statistics.  (Particularly, the frequency of multi-bit errors.)


