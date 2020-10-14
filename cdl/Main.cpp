#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "CDL.hpp"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// Common Variables
int l_CurrentFrame;
int RRCurrentFrame = 0;
bool isPaused = false;
extern retro_environment_t environ_cb;
extern double libRR_playback_speed;

extern "C" {
  long long total_time_elapsed = 0;
  
  void frame_time_callback(retro_usec_t usec) {
    total_time_elapsed += usec;
  }

  bool already_setup_timing = false;
  retro_frame_time_callback frame_cb = {};
  void setup_frame_timing() {
    frame_cb.callback = &frame_time_callback;
    frame_cb.reference = 1000000;
    environ_cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_cb);
    already_setup_timing = true;
  }

  bool libRR_run_frame() {
    if (libRR_playback_speed != 100) {
      // This allows us to control playback speed from the core
      if (!already_setup_timing) {
        setup_frame_timing();
      }
      double fps = 60 * (libRR_playback_speed/100.0);
      double target_time_per_frame = 1000000.0/fps;
      if (total_time_elapsed < target_time_per_frame) {
        return false;
      } 
      total_time_elapsed = 0;
    }

    if (!libRR_settings.paused) {
      RRCurrentFrame++;
    } else {
      sleep(1);
    }
    // printf("Current frame: %d\n", RRCurrentFrame);
    return !libRR_settings.paused;
  }

}