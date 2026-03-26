#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>
#include <zlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char* argv[]){
  png_bytep buffer;
  png_image image;
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  image.opaque = NULL; //demanded by documentation for some reason

  if(png_image_begin_read_from_file(&image, "./small.png") == 0){
    perror("couldn't open and read");
    return 1; 
  }
  image.format = PNG_FORMAT_RGBA;
  buffer = malloc(PNG_IMAGE_SIZE(image));
  printf("size: %d\n", PNG_IMAGE_SIZE(image));
  printf("%d x %d\n", image.width, image.height);

  
  if(png_image_finish_read(&image, NULL, buffer, 0, NULL) ==0){
    perror("couldn't finish reading");
    return 1; 
  }
  
  for(int i = 0; i < PNG_IMAGE_SIZE(image); i+=4){
    if(i % (image.width<<2) == 0 && i > 0)
      printf("\n");
    
    switch((*((uint32_t*)&buffer[i])) & 0xffffff){
    case 0xffffff:
      printf("█");
      break;
    case 0:
      printf(" ");
      break;
    case 0xe1e1ff:
    case 0xe1e1e1:
      printf("▓");
      break;
    case 0xbcbcbc:
      printf("▒");
      break;
    default:
      printf("?");
      break;
    }
  }
  printf("\n");

  
  return 0;
}
