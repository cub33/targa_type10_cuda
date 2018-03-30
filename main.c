#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "ffmpeg.c"
#include "tga.c"

int main(int argc, char const *argv[]) {
  GeneralFrame* frame = importFrame(argv[1]);
  tgaMain(frame);
  return 0;
}
