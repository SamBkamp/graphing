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

  int erm_pause = (image.width * image.height)/255;
  int decrease_by = 0;
  printf("pause: %d\n", erm_pause);

  for(int i = 0; i < image.width * image.height; i++){

    if(i % erm_pause == 0) decrease_by++;

    buffer[i].R = 0xFF - decrease_by;
    buffer[i].G = 0xFF - decrease_by;
    buffer[i].B = 0xFF - decrease_by;
  }
  if(png_image_write_to_file(&image, "output.png", 0, buffer, 0, NULL) == 0 ){
    perror("failed to write");
    return 1;
  }
  printf("final decrease: %d\n", decrease_by);
  return 0;
}
