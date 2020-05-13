#ifndef LIBRETRO_REVERSING_H__
#define LIBRETRO_REVERSING_H__
#include <libretro.h>

#define BitVal(data,y) ( (data>>y) & 1) 


#ifdef __cplusplus
extern "C" {
#endif

// Common Variables
extern int RRCurrentFrame;

// Common functions
extern void show_interface();
extern void log_input_state(retro_input_state_t select_button);
extern void handle_emulator_close();
extern void libRR_handle_load_game();
unsigned long long libRR_playback_next_input_state();
void libRR_run_frame();

// extern "C" {
  void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
// }

#ifdef __cplusplus
}
#endif

#endif