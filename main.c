#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "ffmpeg.c"
#include "tga.c"

int main(int argc, char const *argv[]) {
  VideoFrame* frame = importFrame(argv[1]);
  createTGA(frame);
  return 0;
}
