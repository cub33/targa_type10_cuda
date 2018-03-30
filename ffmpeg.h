
typedef struct rgbPixel {
    int r;
    int g;
    int b;
} RGBPixel;

typedef struct frame {
  int width;
  int height;
  RGBPixel *pixels;
} GeneralFrame;

GeneralFrame* importFrame();

void ffmpegMain(char const *videoPath);
