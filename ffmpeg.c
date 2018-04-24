#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "ffmpeg.h"
/*
compile with command:
gcc -w -o ffmpeg.o ffmpeg.c -lavformat -lavcodec -lswscale -lavutil
*/

VideoFrame *frameToExp; // frame to export

VideoFrame* extractFrame(char const *videoPath) {
  ffmpegMain(videoPath);
  return frameToExp;
};

void printFrame(VideoFrame* frame) {
  printf("width: %d, height: %d\n", frame->width, frame->height);
  int idx = 0;
  uint8_t r, g, b;
  for (int x = 0; x < frame->width; x++)
    for (int y = 0; y < frame->height; y++) {
      idx = y + x * frame->height;
      r = frame->pixels[idx].r;
      g = frame->pixels[idx].g;
      b = frame->pixels[idx].b;
      printf("r: %d, g: %d, b: %d\n", r, g, b);
    }
};

void saveFrame(AVFrame *pFrame, int width, int height) {
  frameToExp = (VideoFrame *) malloc(sizeof(VideoFrame));
  frameToExp->width = width;
  frameToExp->height = height;
  frameToExp->pixels = (RGBPixel *) malloc(sizeof(RGBPixel) * width * height);
  int p, idx = 0;
#if 0
  RGBPixel *pixel = (RGBPixel *) malloc(sizeof(RGBPixel));
  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++) {
      p = x * 3 + y * pFrame->linesize[0];
      idx = y + x * height;
      pixel->r = pFrame->data[0][p];
      pixel->g = pFrame->data[0][p+1];
      pixel->b = pFrame->data[0][p+2];
      frameToExp->pixels[idx] = *pixel;
    }
#else
  memcpy(frameToExp->pixels, pFrame->data[0], sizeof(RGBPixel) * width * height);
#endif
}

void saveFrameToDisk(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];

  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile = fopen(szFilename, "wb");
  if(pFile == NULL)
    exit(0);

  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  // Write pixel data
  int p;
  for(int y = 0; y < height; y++)
    fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width*3, pFile);

  fclose(pFile);
}

void ffmpegMain(char const* videoPath) {
  AVFormatContext *pFormatCtx = NULL;
  AVCodecContext *pCodecCtx = NULL;
  int i = 0;

  /* This registers all available file formats and codecs with the library
  so they will be used automatically when a file with the corresponding
  format/codec is opened */
  av_register_all();

  // open video file
  if(avformat_open_input(&pFormatCtx, videoPath, NULL, NULL) != 0) {
    printf("%s\n", "Could not open the file.");
    exit(0);
  }

  // checking stream info
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0)  {
    printf("%s\n", "Could not find stream information.");
    exit(0);
  }
  /*
  // printing info
  printf("%s\n", "   --------------  File info  --------------\n");
  av_dump_format(pFormatCtx, 0, videoPath, 0);
  printf("%s\n", "\n   --------------  End file info  --------------\n");
  */
  // Find the first video stream
  int videoStream = -1;
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStream = i;
      break;
    }

  if (videoStream == -1) {
    printf("%s\n", "Didn't find a video stream");
    exit(0);
  }
  // Get a pointer to the codec context for the video stream
  pCodecCtx = pFormatCtx->streams[videoStream]->codec;
  AVCodec *pCodec = NULL;

  // Find the decoder for the video stream
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec == NULL) {
    fprintf(stderr, "Unsupported codec\n");
    exit(0);
  }

  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    printf("%s\n", "Could not open codec");
    exit(0);
  }

  // Allocate video frame structures
  AVFrame *pFrame = NULL;
  AVFrame *pFrameRGB = NULL;
  pFrame = av_frame_alloc();
  pFrameRGB = av_frame_alloc();

  // Determine required buffer size and allocate buffer
  int numBytes;
  uint8_t *buffer = NULL;
  numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
  buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

  /* Assign appropriate parts of buffer to image planes in pFrameRGB
  Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  of AVPicture */
  avpicture_fill((AVPicture*) pFrameRGB, buffer,
    AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

  struct SwsContext *sws_ctx = NULL;
  int frameFinished;
  AVPacket packet;
  // initialize SWS context for software scaling
  sws_ctx = sws_getContext(pCodecCtx->width,
    pCodecCtx->height,
    pCodecCtx->pix_fmt,
    pCodecCtx->width,
    pCodecCtx->height,
    AV_PIX_FMT_RGB24,
    SWS_BILINEAR,
    NULL,
    NULL,
    NULL
  );

  i = 0;
  int once = 1;
  while(av_read_frame(pFormatCtx, &packet) >= 0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
    // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

      // Did we get a video frame?
      if(frameFinished) {
        // Convert the image from its native format to RGB
        sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
        pFrame->linesize, 0, pCodecCtx->height,
        pFrameRGB->data, pFrameRGB->linesize);

          // Save the frame to disk
          if(once) {
            once = 0;
            saveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height);
          }
      }
    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  // Free the RGB image
  av_free(buffer);
  av_free(pFrameRGB);

  // Free the YUV frame
  av_free(pFrame);

  // Close the codecs
  avcodec_close(pCodecCtx);

  // Close the video file
  avformat_close_input(&pFormatCtx);

}
