#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "ffmpeg.c"
#include "tga.c"

int main(int argc, char const *argv[]) {
  VideoFrame* frame = extractFrame(argv[1]);
  writeComprTGA(frame);
  //createTGA(frame);
  return 0;
}
