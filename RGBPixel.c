bool valuesAreEqual(RGBPixel value1, RGBPixel value2) {
  bool areEqual = value1.r == value2.r &&
    value1.g == value2.g &&
    value1.b == value2.b;
    return areEqual ? 1 : 0;
}

void printPixel(RGBPixel pixel) {
  printf("| r: %d, g: %d, b: %d | ", pixel.r, pixel.g, pixel.b);
}

void assignRGBValues(RGBPixel* pixel, uint8_t r, uint8_t g, uint8_t b) {
  pixel->r = r; pixel->g = g; pixel->b = b;
}

void copyPixels(RGBPixel* copyTo, RGBPixel* copyFrom) {
  copyTo->r = copyFrom->r;
  copyTo->g = copyFrom->g;
  copyTo->b = copyFrom->b;
}
