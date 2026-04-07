#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <zlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include "graphics.h"

int main(void){
  image_ctx ctx = {720, 720, {720/2, 720/2}, {0x00, 0x00, 0x00}, NULL};
  assert(sizeof(rgb_pixel) == 3);
  rgb_pixel *buffer;
  png_image image;
  rgb_pixel *buffer_2d[ctx.width];
  uint16_t column_size = ctx.width;
  const size_t buffer_size = ctx.width * ctx.height * sizeof(rgb_pixel);

  //libpng
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  image.opaque = NULL; //demanded by the documentation for reasons I don't get
  image.format = PNG_FORMAT_RGB;
  image.width = ctx.width;
  image.height = ctx.height;

  ctx.buffer = buffer_2d;

  buffer = mmap(NULL, buffer_size,
                PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANON,
                -1, 0);
  if(buffer == MAP_FAILED){
    perror("couldn't allocate memory");
    return 1;
  }

  //populate Y axis array for easy access
  for(uint32_t i = 0; i < image.width; i++){
    ctx.buffer[i] = buffer + (column_size*i);
  }

  //populate image with bg colour
  for(uint32_t i = 0; i < ctx.width * ctx.height; i++){
    buffer[i] = ctx.bg_colour;
  }


  uint16_t candle_width = 50;
  uint16_t candle_height = 100;
  uint16_t candle_buffer_left = 20;
  rgb_pixel red = {0xFF, 0, 0};
  pixel_coord rect_center = {candle_buffer_left + (candle_width/2), ctx.center.y};
  rect(rect_center, candle_height, candle_width, red, &ctx);


  if(png_image_write_to_file(&image, "output.png", 0, buffer, 0, NULL) == 0 ){
    perror("failed to write");
    return 1;
  }


  munmap(buffer, buffer_size);
  return 0;
}
