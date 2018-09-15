#ifndef WavFileH
#define WavFileH

#include <stdio.h>

FILE * wavfile_open(const char *filename, unsigned int channels, unsigned int samples_per_sec);
void wavfile_close(FILE * file);

#endif
