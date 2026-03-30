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
  rgb_pixel bg_colour;
}image_ctx;

double pythagoras(double length, double width);
void square(pixel_coord center, uint32_t size, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour);
void circle(pixel_coord center, double radius, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour);
void linear(double gradient, double constant, rgb_pixel **buffer, rgb_pixel colour, pixel_coord center);
void vertical_line(rgb_pixel **buffer, rgb_pixel colour, pixel_coord center);

int main(int argc, char* argv[]){
  assert(sizeof(rgb_pixel) == 3);
  rgb_pixel *buffer;
  png_image image;
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  image.opaque = NULL; //demanded by the documentation for reasons I don't get
  image.format = PNG_FORMAT_RGB;
  image.width = 720;
  image.height = 720;
  rgb_pixel *buffer_2d[image.width];

  uint16_t column_size = image.width;
  const size_t buffer_size = image.width * image.height * sizeof(rgb_pixel);

  buffer = mmap(NULL, buffer_size,
                PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANON,
                -1, 0);
  if(buffer == MAP_FAILED){
    perror("couldn't allocate memory");
    return 1;
  }

  for(uint32_t i = 0; i < image.width; i++){
    buffer_2d[i] = buffer + (column_size*i);
  }

  memset(buffer, 0xFF, buffer_size);
  /*
  //square
  pixel_coord center = {image.width>>1, image.height>>1};
  uint32_t size = 50;
  square(center, size, column_size, buffer, (rgb_pixel){0x00, 0xAA, 0xB7});


  //circle
  pixel_coord center2 = {image.width>>2, image.height>>2};
  double radius = 20;
  rgb_pixel colour = (rgb_pixel){0x00, 0xAA, 0xB7};
  circle(center2, radius, column_size, buffer, colour);
  */

  //line
  rgb_pixel black = {0x00, 0x00, 0x00};
  linear(0, 1, buffer_2d, black, (pixel_coord){image.width/2, image.height/2});
  linear(0, 0, buffer_2d, black, (pixel_coord){image.width/2, image.height/2});
  linear(0, -1, buffer_2d, black, (pixel_coord){image.width/2, image.height/2});

  vertical_line(buffer_2d, black, (pixel_coord){image.width/2, image.height/2});

  rgb_pixel nice_blue = {0x00, 0xAA, 0xB7};
  rgb_pixel dithered_nice_blue = {nice_blue.R + ((0xFF - nice_blue.R)*0.5),
                                  nice_blue.G + ((0xFF - nice_blue.G)*0.5),
                                  nice_blue.B + ((0xFF - nice_blue.B)*0.5)};
  linear(0.5, 0, buffer_2d, nice_blue, (pixel_coord){image.width/2, image.height/2});
  linear(0.5, 1, buffer_2d, nice_blue, (pixel_coord){image.width/2, image.height/2});
  linear(0.5, -1, buffer_2d, nice_blue, (pixel_coord){image.width/2, image.height/2});
  linear(0.5, 2, buffer_2d, dithered_nice_blue, (pixel_coord){image.width/2, image.height/2});
  linear(0.5, -2, buffer_2d, dithered_nice_blue, (pixel_coord){image.width/2, image.height/2});


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

void vertical_line(rgb_pixel **buffer, rgb_pixel colour, pixel_coord center){

  for(uint16_t i = 0; i < 720; i++){
    buffer[i][center.x] = colour;
    buffer[i][center.x+1] = colour;
    buffer[i][center.x-1] = colour;
  }
}

void linear(double gradient, double constant, rgb_pixel **buffer, rgb_pixel colour, pixel_coord center){
  double x = -center.y;
  double y = 0;

  for(; x < center.x; x++){
    y = (gradient*x*-1)-constant;
    if(y >= -center.y && y <= center.y)
      buffer[(uint32_t)y+center.y][(uint32_t)x+center.x] = colour;
  }
}

void circle(pixel_coord center, double radius, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour){
  double start_height = (center.y-radius) * column_size;
  double end_height = (center.y+radius) * column_size;

  for(double i = start_height; i <= end_height; i+= column_size){
    for(double j = center.x - radius; j <= center.x + radius; j++){
      double distance = pythagoras((i/column_size)-center.y, j-center.x);
      if(distance < radius)
        buffer[(int)(i+j)] = colour;
      else{
        double fade_width = 1.2;
        double fade_aggression = 2;
        double vector_scale = fmin(pow(((distance-radius)/fade_width), fade_aggression), 1);
        rgb_pixel colour_diff = {colour.R + (uint8_t)((0xFF-colour.R)*vector_scale),
                                 colour.G + (uint8_t)((0xFF-colour.G)*vector_scale),
                                 colour.B + (uint8_t)((0xFF-colour.B)*vector_scale)};
        buffer[(int)(i+j)] = colour_diff;
      }
    }
  }
}

void square(pixel_coord center, uint32_t size, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour){
  uint32_t start_height = (center.y-(size>>1)) * column_size;
  uint32_t end_height = (center.y+(size>>1)) * column_size;

  for(uint32_t i = start_height; i < end_height; i+= column_size){
    for(uint32_t j = center.x - (size>>1); j < center.x + (size>>1); j++){
      buffer[i+j] = colour;
    }
  }
}
