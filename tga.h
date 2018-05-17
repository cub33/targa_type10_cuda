
typedef struct rle_pkt_struct {
  bool id;
  int repeats;
  RGBPixel value; // to static array
} rle_pkt;

typedef struct raw_pkt_struct {
  bool id;
  int repeats;
  RGBPixel values[128];
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
