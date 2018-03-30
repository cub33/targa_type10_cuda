#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "tga.h"
#include "RGBPixel.c"

void printTGA(TGA tga) {
  for (int i = 0; i < tga.size; i++) {
    Packet pkt = tga.packets[i];
    if (pkt.id == 0) {
      raw_pkt rawPkt = pkt.rawPkt;
      for (int j = 0; j < rawPkt.repeats; j++)
        printPixel(rawPkt.values[j]);
    }
    else {
      rle_pkt rlePkt = pkt.rlePkt;
      printPixel(rlePkt.value);
      printf("x %d, ", rlePkt.repeats);
    }
  }
  printf("\n");
}

void insertRawPkt(TGA* tga, raw_pkt rawPkt) {
  tga->packets[tga->size].rawPkt.repeats = rawPkt.repeats;
  tga->packets[tga->size].rawPkt.values = (RGBPixel *) malloc(sizeof(RGBPixel) * rawPkt.repeats);
  for (int i = 0; i < rawPkt.repeats; i++)
    copyPixels(&(tga->packets[tga->size].rawPkt.values[i]), &rawPkt.values[i]);
  tga->packets[tga->size++].id = 0;
}

void createTGA(VideoFrame* frame) {

  rle_pkt rlePkt = { 1, -1, 0 };
  raw_pkt rawPkt;
  rawPkt.values = (uint8_t *) malloc(sizeof(int) * frame->width); /* FIXME */
  rawPkt.repeats = 0;
  TGA tga;
  tga.packets = (Packet *) malloc(sizeof(Packet) * frame->width * frame->height); /* TODO allocate memory in correct way */
  tga.size = 0;
  bool different = 1;
  int idx = 0;
  for (int x = 0; x < frame->width; x++) {
    for (int y = 0; y < frame->height; y++) {
      idx = y + x * frame->height;
      RGBPixel value, nextValue, prevValue;
      copyPixels(&value, &frame->pixels[idx]);
      assignRGBValues(&nextValue, -1, -1, -1);
      assignRGBValues(&prevValue, -1, -1, -1);
      rlePkt.value = value;
      bool endLine = x == frame->height-1;
      if (x != 0)
        copyPixels(&prevValue, &frame->pixels[idx-1]);
      if (!endLine)
        copyPixels(&nextValue, &frame->pixels[idx+1]);
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
        if (different && !valuesAreEqual(prevValue, value)) {
          copyPixels(&(rawPkt.values[rawPkt.repeats]), &value);
          rawPkt.repeats++;
        }
        if (rawPkt.repeats > 0 && endLine) {
          insertRawPkt(&tga, rawPkt);
          rawPkt.repeats = 0;
        }
        different = 1;
        bool areRepeated = rlePkt.repeats > 0;
        if (areRepeated || (areRepeated && endLine)) {
          rlePkt.repeats++;
          Packet temp;
          copyPixels(&temp.rlePkt.value, &rlePkt.value);
          temp.rlePkt.repeats = rlePkt.repeats;
          tga.packets[tga.size] = temp;
          tga.packets[tga.size++].id = 1;
          assignRGBValues(&rlePkt.value, -1, -1, -1);
          rlePkt.repeats = 0;
        }
      }
    }
  }
  //printTGA(tga);
}
