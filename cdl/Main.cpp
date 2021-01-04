#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "CDL.hpp"
#include <stdlib.h>     //for using the function sleep
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

    // Check if reached end of what player wants to run
    if (libRR_settings.endAt == RRCurrentFrame) {
      // should now check the action to see if we pause or loop
      if (libRR_settings.loopFrame == 0) {
        // restart game
        libRR_reset(0);
        retro_reset();
      }
      else if (libRR_settings.loopFrame > 0) {
        printf("Time to loop back to state: %d\n", libRR_settings.loopFrame);
        libRR_load_save_state(libRR_settings.loopFrame);
        return !libRR_settings.paused;
      } else {
        printf("User wanted to pause after this frame\n");
        libRR_settings.paused = true;
      }
    }

    // Check if paused from the UI
    if (!libRR_settings.paused) {
      RRCurrentFrame++;
    } else {
      #ifdef _WIN32
      Sleep(1);
      #else
      sleep(1);
      #endif
    }
    // printf("Current frame: %d\n", RRCurrentFrame);
    return !libRR_settings.paused;
  }

}