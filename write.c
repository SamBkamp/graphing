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
  uint64_t x;
  uint64_t y;
}pixel_coord;

double pythagoras(double length, double width);
void square(pixel_coord center, uint32_t size, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour);
void circle(pixel_coord center, double radius, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour);

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

  memset(buffer, 0xFF, buffer_size);

  //square
  pixel_coord center = {image.width>>1, image.height>>1};
  uint32_t size = 50;
  square(center, size, column_size, buffer, (rgb_pixel){0x00, 0xAA, 0xB7});

  //circle
  pixel_coord center2 = {image.width>>2, image.height>>2};
  double radius = 20;
  rgb_pixel colour = (rgb_pixel){0x00, 0xAA, 0xB7};
  circle(center2, radius, column_size, buffer, colour);

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

void circle(pixel_coord center, double radius, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour){
  double start_height = (center.y-radius) * column_size;
  double end_height = (center.y+radius) * column_size;

  for(double i = start_height; i <= end_height; i+= column_size){
    for(double j = center.x - radius; j <= center.x + radius; j++){
      double distance = pythagoras((i/column_size)-center.y, j-center.x);
      if(distance < radius)
        buffer[(int)(i+j)] = colour;
      else{
        double vector_scale = fmin(pow(((distance-radius)/1.2), 2), 1);
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
