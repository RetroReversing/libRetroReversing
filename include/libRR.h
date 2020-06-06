#ifndef LIBRETRO_REVERSING_H__
#define LIBRETRO_REVERSING_H__
#include "libretro.h"
#include <string>
using namespace std;

#include "../cdl/nlohmann/json.hpp"
using json = nlohmann::json;

#define BitVal(data,y) ( (data>>y) & 1) 


#ifdef __cplusplus
extern "C" {
#endif

// Common Variables
extern int RRCurrentFrame;
extern bool libRR_should_playback_input;
extern bool libRR_should_append_history;
extern int libRR_should_Load_EPROM;
extern struct retro_memory_descriptor libRR_mmap[];
extern int libRR_mmap_descriptors;
extern struct retro_memory_map libRR_retromap;
extern int libRR_last_logged_frame;

// Input 
extern void libRR_save_button_state_to_file(string filename  = "button_log.bin");
extern void libRR_read_button_state_from_file(string filename  = "button_log.bin", int start_frame=0);

// Scripting support
void libRR_run_script(string code);

// CD
void libRR_add_cd_track(string name, void* data, unsigned int data_length);
void libRR_log_cd_access(int32_t lba);
void libRR_cd_set_able_to_log(bool enable);
struct libRR_cd_track {
  void* data;
  unsigned int length;
  string name;
  bool isData;
};
void to_json(json& j, const libRR_cd_track& p);
extern std::vector<libRR_cd_track> libRR_cd_tracks;

// Screenshots

struct libRR_frame_buffer {
  void* fb;
  unsigned int length;
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
};
extern libRR_frame_buffer libRR_current_frame_buffer;
void libRR_create_png(string filename, libRR_frame_buffer libRR_current_frame_buffer);
void libRR_set_framebuffer(const void *fb, unsigned int length, unsigned int width, unsigned int height, unsigned int pitch);


// 
// Structures
// 
struct libRR_save_state {
  string name;
  int frame;
};

// 
// File Functions
// 
bool libRR_write_binary_data_to_file(uint8_t * data, size_t len, string file_name);
bool libRR_read_binary_data_from_file(uint8_t * data, size_t len, string file_name);

// 
// Functions
// 
void libRR_reset(unsigned int reset_frame);
extern void show_interface();
extern void log_input_state(retro_input_state_t select_button);
void libRR_log_input_state_bitmask(retro_input_state_t input_cb);
retro_input_state_t libRR_handle_input(retro_input_state_t input_cb);
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