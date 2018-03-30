#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "tga.h"
//#include "ffmpeg.h"

/*
#define WIDTH 5
#define HEIGHT 6

TODO: change type of rgb variables in uint8_t

int img[WIDTH][HEIGHT][3] = {
 {{1, 3, 4}, {8, 3, 4}, {1, 3, 4}, {2, 3, 4}, {3, 3, 4}, {9, 3, 4}} ,
 {{7, 3, 4}, {3, 3, 4}, {5, 3, 4}, {2, 3, 4}, {7, 3, 4}, {6, 3, 4}} ,
 {{5, 3, 4}, {4, 3, 4}, {2, 3, 4}, {5, 3, 4}, {4, 3, 4}, {1, 3, 4}} ,
 {{5, 3, 4}, {5, 3, 4}, {2, 3, 4}, {5, 3, 4}, {5, 3, 4}, {2, 3, 4}} ,
 {{4, 3, 4}, {2, 3, 4}, {2, 3, 4}, {5, 3, 4}, {5, 3, 4}, {3, 3, 4}}
};
*/

void printTGA(TGA tga) {
  for (int i = 0; i < tga.size; i++) {
    Packet pkt = tga.packets[i];
    if (pkt.id == 0) {
      raw_pkt rawPkt = pkt.rawPkt;
      for (int j = 0; j < rawPkt.repeats; j++)
        printf("%d, ", rawPkt.values[j]);
    }
    else {
      rle_pkt rlePkt = pkt.rlePkt;
      printf("%d x %d, ", rlePkt.value, rlePkt.repeats);
    }
  }
  printf("\n");
}

void insertRawPkt(TGA* tga, raw_pkt rawPkt) {
  tga->packets[tga->size].rawPkt.repeats = rawPkt.repeats;
  tga->packets[tga->size].rawPkt.values = (int*) malloc(sizeof(int) * rawPkt.repeats);
  for (int i = 0; i < rawPkt.repeats; i++)
    tga->packets[tga->size].rawPkt.values[i] = rawPkt.values[i];
  tga->packets[tga->size++].id = 0;
}

bool valuesAreEqual(RGBPixel* value1, RGBPixel* value2) {
  bool equal = value1->r == value2->r &&
    value1->g == value2->g &&
    value1->b == value2->b;
    return equal ? 1 : 0;
}

void assignRGBValues(RGBPixel* pixel, uint8_t r, uint8_t g, uint8_t b) {
  pixel->r = r; pixel->g = g; pixel->b = b;
}

void copyPixels(RGBPixel* copyTo, RGBPixel* copyFrom) {
  copyTo->r = copyFrom->r;
  copyTo->g = copyFrom->g;
  copyTo->b = copyFrom->b;
}

void tgaMain(GeneralFrame* frame) {

  rle_pkt rlePkt = { 1, -1, 0 };
  raw_pkt rawPkt;
  rawPkt.values = (int*) malloc(sizeof(int) * frame->width); /* FIXME */
  rawPkt.repeats = 0;
  TGA tga;
  tga.packets = (Packet*) malloc(sizeof(Packet) * 50000000); /* TODO allocate memory in correct way */
  tga.size = 0;
  bool different = 1;
  int idx = 0;
  for (int x = 0; x < frame->width; x++) {
    for (int y = 0; y < frame->height; y++) {
      idx = y + x * frame->height;
      RGBPixel* value, nextValue, prevValue;
      copyPixels(value, frame->pixels[idx])
      /**value = assignRGBValues(
        frame->pixels[idx].r,
        frame->pixels[idx].g,
        frame->pixels[idx].b
      );*/
      assignRGBValues(nextValue, -1, -1, -1);
      assignRGBValues(prevValue, -1, -1, -1);
      rlePkt.value = value;
      bool endLine = x == frame->height-1;
      if (x != 0)
        copyPixels(prevValue, *frame->pixels[idx-1]);
        //prevValue = frame->pixels[idx-1].r;
      if (!endLine)
        copyPixels(nextValue, *frame->pixels[idx+1]);
        //nextValue = frame->pixels[idx+1].r;
      else
        different = 1;
      if (valuesAreEqual(value, nextValue)) {
        rlePkt.repeats++;
        different = 0;
        if (rawPkt.repeats > 0) {
          insertRawPkt(&tga, rawPkt);
          rawPkt.repeats = 0;
        }
      }
      else {
        if (different && !valuesAreEqual(prevValue, value) /*prevValue != value*/)
          rawPkt.values[rawPkt.repeats++] = value;
        if (rawPkt.repeats > 0 && endLine) {
          insertRawPkt(&tga, rawPkt);
          rawPkt.repeats = 0;
        }
        different = 1;
        bool areRepeated = rlePkt.repeats > 0;
        if (areRepeated || (areRepeated && endLine)) {
          rlePkt.repeats++;
          Packet temp;
          temp.rlePkt = rlePkt;
          tga.packets[tga.size] = temp;
          tga.packets[tga.size++].id = 1;
          rlePkt.value = -1;
          rlePkt.repeats = 0;
        }
      }
    }
  }
  printTGA(tga);
}

/*
int main(int argc, char const *argv[]) {

  return 0;
}
*/
