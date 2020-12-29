#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../png/lodepng.h"
#include "CDL.hpp"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

extern "C"
{
  enum retro_pixel_format libRR_core_pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;

  void libRR_create_png(string filename, libRR_frame_buffer libRR_current_frame_buffer)
  {
    unsigned int pitch = libRR_current_frame_buffer.pitch;
    unsigned int width = libRR_current_frame_buffer.width;
    unsigned int height = libRR_current_frame_buffer.height;
    unsigned char *data = (unsigned char *)libRR_current_frame_buffer.fb;
    if (data == NULL)
    {
      printf("Warning data was NULL \n");
      return;
    }
    std::vector<unsigned char> image;
    int png_bytes_per_pixel = 4;
    image.resize(width * height * png_bytes_per_pixel);

    if (libRR_core_pixel_format == RETRO_PIXEL_FORMAT_XRGB8888) {

      for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++)
        {
          image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 0] = data[pitch * y + png_bytes_per_pixel * x + 2]; // Red
          image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 1] = data[pitch * y + png_bytes_per_pixel * x + 1]; // Green
          image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 2] = data[pitch * y + png_bytes_per_pixel * x + 0]; // Blue
          image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 3] = 255; // Alpha
        }
      }
    } else if (libRR_core_pixel_format == RETRO_PIXEL_FORMAT_RGB565) {
      pitch = pitch/2;
      for (unsigned y = 0; y < height; y++) {

          for (unsigned x = 0; x < width; x++)
          {
            uint16_t pixel = ((uint16_t*)data)[pitch * y + 1 * x + 0];
            uint8_t b = (pixel >>  0) & 0x1f;
            uint8_t g = (pixel >>  5) & 0x3f;
            uint8_t r = (pixel >> 11) & 0x1f;

            image[(png_bytes_per_pixel * width * y) + (png_bytes_per_pixel * x) + 0] = (r << 3) | (r >> 2); // Red
            image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 1] = (g << 2) | (g >> 4); // Green
            image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 2] =  (b << 3) | (b >> 2); // Blue
            image[png_bytes_per_pixel * width * y + png_bytes_per_pixel * x + 3] = 255; // Alpha
            
            }
      }

    } else {
      printf("ERROR: Can't generate PNG due to unsupported pixel formart in core\n");
    }

    
    //Encode the image
    unsigned error = lodepng::encode(filename, image, width, height);

    //if there's an error, display it
    if (error)
      std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
  }
}