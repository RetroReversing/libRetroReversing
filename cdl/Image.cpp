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
    image.resize(width * height * 4);
    // image.assign(data, data+(width*height*4));
    for (unsigned y = 0; y < height; y++)
      for (unsigned x = 0; x < width; x++)
      {
        image[4 * width * y + 4 * x + 0] = data[pitch * y + 4 * x + 2];
        image[4 * width * y + 4 * x + 1] = data[pitch * y + 4 * x + 1];
        image[4 * width * y + 4 * x + 2] = data[pitch * y + 4 * x + 0];
        image[4 * width * y + 4 * x + 3] = 255;
      }
    //Encode the image
    unsigned error = lodepng::encode(filename, image, width, height);

    //if there's an error, display it
    if (error)
      std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
  }
}