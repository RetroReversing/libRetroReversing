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

extern "C" {

  bool libRR_run_frame() {
    if (!libRR_settings.paused) {
      RRCurrentFrame++;
    } else {
      sleep(1);
    }
    // printf("Current frame: %d\n", RRCurrentFrame);
    return !libRR_settings.paused;
  }

}