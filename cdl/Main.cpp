#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "CDL.hpp"

// Common Variables
int l_CurrentFrame;
int RRCurrentFrame;
bool isPaused = false;

extern "C" {

  bool libRR_run_frame() {
    RRCurrentFrame++;
    // printf("Current frame: %d\n", RRCurrentFrame);
    return !libRR_settings.paused;
  }

}