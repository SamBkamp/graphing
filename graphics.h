#ifndef GRAPHICS_HEADER
#define GRAPHICS_HEADER

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
  rgb_pixel *raw_buffer;
}image_ctx;

image_ctx *create_context(uint32_t height, uint32_t width, rgb_pixel colour);
double pythagoras(double length, double width);
void square(pixel_coord center, uint32_t size, rgb_pixel colour, image_ctx *ctx);
void circle(pixel_coord center, double radius, rgb_pixel colour, image_ctx *ctx);
void clamped_linear(int32_t start, int32_t end, double gradient, double constant, rgb_pixel colour, int32_t width, image_ctx *ctx);
void clamped_vertical_line(int32_t starty, int32_t endy, rgb_pixel colour, image_ctx *ctx, int32_t constant);
void point(pixel_coord p, rgb_pixel colour, image_ctx *ctx);
void quadratic(double quadratic_c, int32_t linear_c, int32_t constant, rgb_pixel colour, image_ctx *ctx);
void rect(pixel_coord center, uint32_t height, uint32_t width, rgb_pixel colour, image_ctx *ctx);

#endif
