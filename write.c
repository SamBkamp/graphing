#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <zlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

typedef struct{
  uint8_t R;
  uint8_t G;
  uint8_t B;
}rgb_pixel;

typedef struct{
  int32_t x;
  int32_t y;
}pixel_coord;

typedef struct{
  uint32_t width;
  uint32_t height;
  pixel_coord center;
  rgb_pixel bg_colour;
  rgb_pixel **buffer;
}image_ctx;

double pythagoras(double length, double width);
void square(pixel_coord center, uint32_t size, rgb_pixel colour, image_ctx *ctx);
void circle(pixel_coord center, double radius, rgb_pixel colour, image_ctx *ctx);
void linear(double gradient, double constant, rgb_pixel colour, image_ctx *ctx);
void vertical_line(rgb_pixel colour, image_ctx *ctx);

int main(int argc, char* argv[]){
  image_ctx ctx = {720, 720, {720/2, 720/2}, {0xFF, 0xFF, 0xFF}, NULL};
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

  //populate Y axis
  for(uint32_t i = 0; i < image.width; i++){
    ctx.buffer[i] = buffer + (column_size*i);
  }

  //populate image with bg colour
  for(uint32_t i = 0; i < ctx.width * ctx.height; i++){
    buffer[i] = ctx.bg_colour;
  }


  //square
  /*
  pixel_coord center = {image.width>>1, image.height>>1};
  uint32_t size = 50;
  square(center, size, (rgb_pixel){0x00, 0xAA, 0xB7}, &ctx);


  //circle
  pixel_coord center2 = {image.width>>2, image.height>>2};
  double radius = 20;
  rgb_pixel colour = (rgb_pixel){0x00, 0xAA, 0xB7};
  circle(center2, radius, colour, &ctx);
  */

  //line
  rgb_pixel black = {0x00, 0x00, 0x00};
  linear(0, 1, black, &ctx);
  linear(0, 0, black, &ctx);
  linear(0, -1, black, &ctx);

  vertical_line(black, &ctx);

  rgb_pixel nice_blue = {0x00, 0xAA, 0xB7};
  rgb_pixel dithered_nice_blue = {nice_blue.R + ((0xFF - nice_blue.R)*0.5),
                                  nice_blue.G + ((0xFF - nice_blue.G)*0.5),
                                  nice_blue.B + ((0xFF - nice_blue.B)*0.5)};
  linear(0.5, 1, nice_blue, &ctx);
  linear(0.5, 0, nice_blue, &ctx);
  linear(0.5, -1, nice_blue, &ctx);
  linear(0.5, 2, dithered_nice_blue, &ctx);
  linear(0.5, -2, dithered_nice_blue, &ctx);


  if(png_image_write_to_file(&image, "output.png", 0, buffer, 0, NULL) == 0 ){
    perror("failed to write");
    return 1;
  }


  munmap(buffer, buffer_size);
  return 0;
}

double pythagoras(double length, double width){
  //ret^2 = sqrt(length^2+width^2)
  double ret = pow(length, 2) + pow(width, 2);
  return sqrt(ret);

}

void vertical_line(rgb_pixel colour, image_ctx *ctx){

  for(uint16_t i = 0; i < 720; i++){
    ctx->buffer[i][ctx->center.x] = colour;
    ctx->buffer[i][(ctx->center.x)+1] = colour;
    ctx->buffer[i][(ctx->center.x)-1] = colour;
  }
}

void linear(double gradient, double constant, rgb_pixel colour, image_ctx *ctx){
  double x = -ctx->center.y;
  double y = 0;

  for(; x < ctx->center.x; x++){
    y = (gradient*x*-1)-constant;
    if(y >= -ctx->center.y && y <= ctx->center.y)
      ctx->buffer[(uint32_t)y+ctx->center.y][(uint32_t)x+ctx->center.x] = colour;
  }
}

void circle(pixel_coord center, double radius, rgb_pixel colour, image_ctx *ctx){

  for(double i = center.y - radius; i <= center.y + radius; i++){
    for(double j = center.x - radius; j <= center.x + radius; j++){
      double distance = pythagoras(i-center.y, j-center.x);
      if(distance < radius)
        ctx->buffer[(int)i][(int)j] = colour;
      else{
        double fade_width = 1.2;
        double fade_aggression = 2;
        double vector_scale = fmin(pow(((distance-radius)/fade_width), fade_aggression), 1);
        rgb_pixel colour_diff = {colour.R + (uint8_t)((0xFF-colour.R)*vector_scale),
                                 colour.G + (uint8_t)((0xFF-colour.G)*vector_scale),
                                 colour.B + (uint8_t)((0xFF-colour.B)*vector_scale)};
        ctx->buffer[(int)i][(int)j] = colour_diff;
      }
    }
  }
}

void square(pixel_coord center, uint32_t size, rgb_pixel colour, image_ctx *ctx){

  for(uint32_t i = center.y-(size/2); i < center.y+(size/2); i++){
    for(uint32_t j = center.x-(size/2); j < center.x+(size/2); j++){
      ctx->buffer[i][j] = colour;
    }
  }
}
