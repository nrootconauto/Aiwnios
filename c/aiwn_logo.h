#pragma once

extern const struct CLogo {
  unsigned int width;
  unsigned int height;
  unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char compressed_pixel_data[6556];
} aiwnios_logo;
