#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#define WIDTH 5
#define HEIGHT 6

/* TODO: Pixel struct */

typedef struct rle_pkt_struct {
  bool id;
  /* Pixel */int value;
  int repeats;
} rle_pkt;

typedef struct raw_pkt_struct {
  bool id;
  /* Pixel* */ int* values;
  int repeats;
} raw_pkt;

typedef union {
  bool id;
  rle_pkt rlePkt;
  raw_pkt rawPkt;
} Packet;

typedef struct tga_struct {
  Packet* packets;
  int size;
} TGA;

int img[WIDTH][HEIGHT][3] = {
 {{1, 3, 4}, {8, 3, 4}, {1, 3, 4}, {2, 3, 4}, {3, 3, 4}, {9, 3, 4}} ,
 {{7, 3, 4}, {3, 3, 4}, {5, 3, 4}, {2, 3, 4}, {7, 3, 4}, {6, 3, 4}} ,
 {{5, 3, 4}, {4, 3, 4}, {2, 3, 4}, {5, 3, 4}, {4, 3, 4}, {1, 3, 4}} ,
 {{5, 3, 4}, {5, 3, 4}, {2, 3, 4}, {5, 3, 4}, {5, 3, 4}, {2, 3, 4}} ,
 {{4, 3, 4}, {2, 3, 4}, {2, 3, 4}, {5, 3, 4}, {5, 3, 4}, {3, 3, 4}}
};

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
  raw_pkt _rawPkt = tga->packets[tga->size].rawPkt;
  tga->packets[tga->size++].id = 0;
}

int main(int argc, char const *argv[]) {
  rle_pkt rlePkt = { 1, -1, 0 };
  raw_pkt rawPkt;
  rawPkt.values = (int*) malloc(sizeof(int)*WIDTH);
  rawPkt.repeats = 0;
  TGA tga;
  tga.packets = (Packet*) malloc(sizeof(Packet)*500); /* TODO allocate memory in correct way */
  tga.size = 0;
  bool different = 1;
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int r = img[x][y][0];
      //int g = img[x][y][1];
      //int b = img[x][y][2];
      int value = r;
      rlePkt.value = value;
      bool endLine = img[x+1][y][0] == 0;
      int nextValue = -1;
      int prevValue = -1;
      if (x != 0)
        prevValue = img[x-1][y][0];
      if (!endLine)
        nextValue = img[x+1][y][0];
      else
        different = 1;
      if (value == nextValue) {
        rlePkt.repeats++;
        different = 0;
        if (rawPkt.repeats > 0) {
          insertRawPkt(&tga, rawPkt);
          rawPkt.repeats = 0;
        }
      }
      else {
        if (different && prevValue != value)
          rawPkt.values[rawPkt.repeats++] = value;
        if (rawPkt.repeats > 0 && endLine) {
          insertRawPkt(&tga, rawPkt);
          rawPkt.repeats = 0;
        }
        different = 1;
        bool areRepeated = rlePkt.repeats > 0;
        if (areRepeated || (areRepeated && endLine)) {
          rlePkt.repeats++;
          tga.packets[tga.size] = (Packet) rlePkt;
          tga.packets[tga.size++].id = 1;
          rlePkt.value = -1;
          rlePkt.repeats = 0;
        }
      }
    }
  }
  printTGA(tga);

  return 0;
}
