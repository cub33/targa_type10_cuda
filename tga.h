
typedef struct rle_pkt_struct {
  bool id;
  /* Pixel */RGBPixel value;
  int repeats;
} rle_pkt;

typedef struct raw_pkt_struct {
  bool id;
  /* Pixel* */ RGBPixel* values;
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
