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

void square(pixel_coord center, uint32_t size, uint32_t column_size, rgb_pixel *buffer, rgb_pixel colour);

int main(int argc, char* argv[]){
  assert(sizeof(rgb_pixel) == 3);
  rgb_pixel *buffer;
  png_image image;
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  image.opaque = NULL;
  image.format = PNG_FORMAT_RGB;
  image.width = 720;
  image.height = 720;

  uint16_t column_size = image.width;

  size_t buffer_size = image.width * image.height * sizeof(rgb_pixel);
  buffer = mmap(NULL, buffer_size,
                PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANON,
                -1, 0);
  if(buffer == MAP_FAILED){
    perror("couldn't allocate memory");
    return 1;
  }else{
    printf("address: %p\nsize: %ld\n", buffer, buffer_size);
  }

  memset(buffer, 0xFF, buffer_size);

  //square
  pixel_coord center = {image.width>>1, image.height>>1};
  pixel_coord center2 = {image.width>>2, image.height>>2};
  uint32_t size = 50;
  square(center, size, column_size, buffer, (rgb_pixel){0x00, 0xAA, 0xB7});
  square(center2, 25, column_size, buffer, (rgb_pixel){0x00, 0x00, 0x00});

  if(png_image_write_to_file(&image, "output.png", 0, buffer, 0, NULL) == 0 ){
    perror("failed to write");
    return 1;
  }

  return 0;
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
