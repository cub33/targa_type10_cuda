#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "tga.h"
#include "RGBPixel.c"
TGA createTGA(VideoFrame* frame);

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

void writeComprTGA(VideoFrame* frame) {
  TGA tga = createTGA(frame);
  int width = frame->width;
  int height = frame->height;
  FILE *fptr;
  fptr = fopen("compressed.tga", "wb");
  /* writing header */
  putc(0,fptr);
  putc(0,fptr);
  putc(10,fptr);                        /* compressed RGB */
  putc(0,fptr); putc(0,fptr);
  putc(0,fptr); putc(0,fptr);
  putc(0,fptr);
  putc(0,fptr); putc(0,fptr);           /* X origin */
  putc(0,fptr); putc(0,fptr);           /* y origin */
  putc((width & 0x00FF),fptr);
  putc((width & 0xFF00) / 256,fptr);
  putc((height & 0x00FF),fptr);
  putc((height & 0xFF00) / 256,fptr);
  putc(24,fptr);                        /* 24 bit bitmap */
  putc(0,fptr);
  /* writing image data */
  for (int i = 0; i < tga.size; i++) {
    Packet pkt = tga.packets[i];
    unsigned char pktHeader;
    if (pkt.id == 0) {
      raw_pkt rawPkt = pkt.rawPkt;
      if (rawPkt.repeats > 128) {
        exit(1);
      }
      pktHeader = rawPkt.repeats - 1;
      putc(pktHeader, fptr);
      for (int j = 0; j < rawPkt.repeats; j++) {
        putc(rawPkt.values[j].b, fptr);
        putc(rawPkt.values[j].g, fptr);
        putc(rawPkt.values[j].r, fptr);
      }
    }
    else {
      rle_pkt rlePkt = pkt.rlePkt;
      if (rlePkt.repeats > 128) {
        exit(2);
      }
      pktHeader = (rlePkt.repeats - 1) + 128 ;
      putc(pktHeader, fptr);
      putc(rlePkt.value.b, fptr);
      putc(rlePkt.value.g, fptr);
      putc(rlePkt.value.r, fptr);
    }
  }
  fclose(fptr);
}

void writeUncomprTGA(VideoFrame* frame) {
  int width = frame->width;
  int height = frame->height;
  FILE *fptr;
  fptr = fopen("uncompressed.tga", "wb");
  /* writing header */
  putc(0,fptr);
  putc(0,fptr);
  putc(2,fptr);                         /* uncompressed RGB */
  putc(0,fptr); putc(0,fptr);
  putc(0,fptr); putc(0,fptr);
  putc(0,fptr);
  putc(0,fptr); putc(0,fptr);           /* X origin */
  putc(0,fptr); putc(0,fptr);           /* y origin */
  putc((width & 0x00FF),fptr);
  putc((width & 0xFF00) / 256,fptr);
  putc((height & 0x00FF),fptr);
  putc((height & 0xFF00) / 256,fptr);
  putc(24,fptr);                        /* 24 bit bitmap */
  putc(0,fptr);
  /* writing image data */
  for (int i = 0; i < height * width; i++) {
     putc(frame->pixels[i].b, fptr);
     putc(frame->pixels[i].g, fptr);
     putc(frame->pixels[i].r, fptr);
  }
  fclose(fptr);
}

TGA createTGA(VideoFrame* frame) {
  rle_pkt rlePkt = { 1, -1, 0 };
  raw_pkt rawPkt;
  rawPkt.values = (uint8_t *) malloc(sizeof(int) * frame->width); /* FIXME */
  rawPkt.repeats = 0;
  TGA tga;
  tga.packets = (Packet *) malloc(sizeof(Packet) * frame->width * frame->height); /* TODO allocate memory in correct way */
  tga.size = 0;
  bool different = 1;
  int idx = 0;
  for (int y = 0; y < frame->height; y++) {
    for (int x = 0; x < frame->width; x++) {
      idx = x + y * frame->width-1;
      RGBPixel value, nextValue, prevValue;
      copyPixels(&value, &frame->pixels[idx]);
      assignRGBValues(&nextValue, -1, -1, -1);
      assignRGBValues(&prevValue, -1, -1, -1);
      rlePkt.value = value;
      bool endLine = x == frame->width-1;
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
  printTGA(tga);
  return tga;
}

void insertRawPkt(TGA* tga, raw_pkt rawPkt) {
  tga->packets[tga->size].rawPkt.repeats = rawPkt.repeats;
  tga->packets[tga->size].rawPkt.values = (RGBPixel *) malloc(sizeof(RGBPixel) * rawPkt.repeats);
  for (int i = 0; i < rawPkt.repeats; i++)
    copyPixels(&(tga->packets[tga->size].rawPkt.values[i]), &rawPkt.values[i]);
  tga->packets[tga->size++].id = 0;
}
