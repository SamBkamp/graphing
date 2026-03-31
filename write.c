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
void clamped_linear(int32_t start, int32_t end, double gradient, double constant, rgb_pixel colour, int32_t width, image_ctx *ctx);
void clamped_vertical_line(int32_t starty, int32_t endy, rgb_pixel colour, image_ctx *ctx, int32_t constant);

int main(void){
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

  uint8_t tick_length = 20;
  int32_t pixels_per_int = ctx.width/10;
  rgb_pixel grey = {0xee, 0xee, 0xee};
  rgb_pixel black = {0x00, 0x00, 0x00};
  //grid lines
  //X direction
  for(int32_t i = -(ctx.width/2); i < (int32_t)(ctx.width/2); i+=pixels_per_int){
    clamped_linear(-(ctx.width/2), (ctx.width/2), 0, i, grey, 1, &ctx); //line
    clamped_linear(-(tick_length>>1), tick_length>>1, 0, i, black, 1, &ctx); //notches
  }
  //Y direction
  for(int32_t i = -(ctx.width/2)+pixels_per_int; i < (int32_t)(ctx.width/2); i+=pixels_per_int){
    clamped_vertical_line(-(ctx.height/2), (ctx.height/2), grey, &ctx, i);
    clamped_vertical_line(-(tick_length>>1), tick_length>>1, black, &ctx, i);
  }


  //x and y axis
  clamped_linear(-(ctx.height/2), (ctx.height/2), 0, 0, black, 1, &ctx);

  clamped_vertical_line(-(ctx.height/2), (ctx.height/2), black, &ctx, 0);



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


  rgb_pixel nice_blue = {0x00, 0xAA, 0xB7};

  clamped_linear(-(ctx.width/2), ctx.width/2, 0.5, ctx.height*0.1, nice_blue, 1, &ctx);

  clamped_linear(-50, 50, 0.5, 0, nice_blue, 1, &ctx);

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

void clamped_vertical_line(int32_t starty, int32_t endy, rgb_pixel colour, image_ctx *ctx, int32_t constant){
  int16_t low = (int16_t)fmin(ctx->center.y-starty, ctx->center.y-endy);
  int16_t hi = (int16_t)fmax(ctx->center.y-starty, ctx->center.y-endy);
  for(int16_t i = low; i < hi; i++){
    ctx->buffer[i][ctx->center.x+constant] = colour;
    ctx->buffer[i][(ctx->center.x)+1+constant] = colour;
    ctx->buffer[i][(ctx->center.x)-1+constant] = colour;
  }
}

void clamped_linear(int32_t start, int32_t end, double gradient, double constant, rgb_pixel colour, int32_t width, image_ctx *ctx){
  double x = start;
  double y = 0;
  for(; x < end; x++){
    y = (gradient*x*-1)-constant;
    for(double i = y-width; i <= y+width; i++){
      if(i+ctx->center.y >= 0 && i+ctx->center.y < (int32_t)ctx->width){
        ctx->buffer[(uint32_t)roundf(i+ctx->center.y)][(uint32_t)x+ctx->center.x] = colour;
      }
    }

    double dithered_d_lo = roundf(y-width-1+ctx->center.y);
    double dithered_d_hi = roundf(y+width+1+ctx->center.y);
    if(dithered_d_lo >= 0 && dithered_d_lo < (int32_t) ctx->width && gradient != 0){
      ctx->buffer[(uint32_t)dithered_d_lo][(uint32_t)x+ctx->center.x] =
        (rgb_pixel){colour.R + ((ctx->buffer[(uint32_t)dithered_d_lo][(uint32_t)x+ctx->center.x].R - colour.R)*0.5),
                    colour.G + ((ctx->buffer[(uint32_t)dithered_d_lo][(uint32_t)x+ctx->center.x].G - colour.G)*0.5),
                    colour.B + ((ctx->buffer[(uint32_t)dithered_d_lo][(uint32_t)x+ctx->center.x].B - colour.B)*0.5)};
    }

    if(dithered_d_hi >= 0 && dithered_d_hi < (int32_t)ctx->width && gradient != 0){
      ctx->buffer[(uint32_t)dithered_d_hi][(uint32_t)x+ctx->center.x] =
        (rgb_pixel){colour.R + ((ctx->buffer[(uint32_t)dithered_d_hi][(uint32_t)x+ctx->center.x].R - colour.R)*0.5),
                    colour.G + ((ctx->buffer[(uint32_t)dithered_d_hi][(uint32_t)x+ctx->center.x].G - colour.G)*0.5),
                    colour.B + ((ctx->buffer[(uint32_t)dithered_d_hi][(uint32_t)x+ctx->center.x].B - colour.B)*0.5)};
    }

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
