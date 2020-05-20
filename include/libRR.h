#ifndef LIBRETRO_REVERSING_H__
#define LIBRETRO_REVERSING_H__
#include "libretro.h"
#include <string>
using namespace std;

#define BitVal(data,y) ( (data>>y) & 1) 


#ifdef __cplusplus
extern "C" {
#endif

// Common Variables
extern int RRCurrentFrame;
extern int libRRshouldLogInput;
extern int libRR_should_Load_EPROM;
extern struct retro_memory_descriptor libRR_mmap[];
extern int libRR_mmap_descriptors;
extern struct retro_memory_map libRR_retromap;

// Input 
extern void libRR_save_button_state_to_file();
extern void libRR_read_button_state_from_file();

// 
// Structures
// 
struct libRR_save_state {
  string name;
  int frame;
};

// 
// Functions
// 
extern void show_interface();
extern void log_input_state(retro_input_state_t select_button);
// extern void handle_emulator_close();
extern void libRR_handle_load_game(const struct retro_game_info *info, retro_environment_t environ_cb);
void libRR_display_message(const char *format, ...);

// Input
unsigned long long libRR_playback_next_input_state();

// Main
bool libRR_run_frame();

// Web
string libRR_parse_message_from_web(string message);

// Override with Console specific
void libRR_setup_console_details(retro_environment_t environ_cb);
void libRR_handle_emulator_close();

// extern "C" {
  void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
// }

#ifdef __cplusplus
}
#endif

#endif