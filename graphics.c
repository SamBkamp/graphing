#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include "graphics.h"

image_ctx *create_context(uint32_t height, uint32_t width, rgb_pixel colour){
  image_ctx *ret = malloc(sizeof(image_ctx));

  ret->width = width;
  ret->height = height;
  ret->center = (pixel_coord){width/2, height/2};
  ret->buffer = malloc(sizeof(ret->buffer)*width);
  ret->bg_colour = colour;
  ret->raw_buffer = mmap(NULL, height*width*sizeof(rgb_pixel),
                         PROT_READ|PROT_WRITE,
                         MAP_SHARED|MAP_ANON,
                         -1, 0);

  if(ret->raw_buffer == MAP_FAILED){
    perror("couldn't allocate memory");
    return NULL;
  }

  //populate columns variable
  for(uint32_t i = 0; i < width; i++){
    ret->buffer[i] = ret->raw_buffer + (width*i);
  }

  return ret;
}

double pythagoras(double length, double width){
  //ret^2 = sqrt(length^2+width^2)
  double ret = pow(length, 2) + pow(width, 2);
  return sqrt(ret);

}

void point(pixel_coord p, rgb_pixel colour, image_ctx *ctx){
  circle(p, 5, colour, ctx);
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

  for(double i = (ctx->center.y-center.y) - radius; i <= (ctx->center.y-center.y) + radius; i++){
    for(double j = (center.x+ctx->center.x) - radius; j <= (center.x+ctx->center.x) + radius; j++){
      double distance = pythagoras(i-(ctx->center.y-center.y), j-(ctx->center.y+center.x));
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

void rect(pixel_coord center, uint32_t height, uint32_t width, rgb_pixel colour, image_ctx *ctx){

  for(uint32_t i = center.y-(height/2); i < center.y+(height/2); i++){
    for(uint32_t j = center.x-(width/2); j < center.x+(width/2); j++){
      ctx->buffer[i][j] = colour;
    }
  }
}


void quadratic(double quadratic_c, int32_t linear_c, int32_t constant, rgb_pixel colour, image_ctx *ctx){
  double x = -(double)ctx->width;
  double y = 0;
  long double scale = 1/7.2;
  //printf("scale = %Lf\n", scale);
  for(; x < ctx->width; x+=1){
    y = (-(pow(quadratic_c*x, 2)) * (scale/10))-(linear_c*x)-constant;
    //printf("x: %f, y: %f\n", x, y);
    for(double i = y-2; i <= y+2; i++){
      if(i+ctx->center.y >= 0 && i+ctx->center.y < (int32_t)ctx->width){
        ctx->buffer[(uint32_t)roundf(i+ctx->center.y)][(uint32_t)x+ctx->center.x] = colour;
      }
    }
  }
}
