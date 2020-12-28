#ifndef LIBRETRO_REVERSING_H__
#define LIBRETRO_REVERSING_H__
#include "libretro.h"
#include <string>
using namespace std;
#include "./libRR_c.h"

#include "../cdl/nlohmann/json.hpp"
using json = nlohmann::json;

#define BitVal(data,y) ( (data>>y) & 1) 

void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int arguments=0);
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int arguments, unsigned m, unsigned n);
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int arguments, unsigned m, unsigned n, unsigned imm, unsigned d);
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int arguments, unsigned m, unsigned n, unsigned imm, unsigned d, unsigned ea);

string libRR_get_data_for_function(int offset, int length, bool swapEndian, bool asHexString);

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
extern const char* libRR_console;

// Game Name
extern string libRR_game_name;

// 
// Paths
// 
extern string libRR_project_directory;
extern string libRR_export_directory;

// whether we have finished the boot rom process or not
extern bool libRR_finished_boot_rom;

// JSON
extern json game_json;
// extern json function_playthough_info; //function_usage instead
extern json libRR_disassembly;
extern json libRR_memory_reads;
extern json libRR_rom_reads;
extern json libRR_consecutive_memory_reads;
extern json libRR_consecutive_rom_reads;
extern json libRR_called_functions;
extern json libRR_long_jumps;
extern int32_t previous_consecutive_rom_read; // previous read address to check if this read is part of the chain
extern int16_t previous_consecutive_rom_bank; // previous read address to check if this read is part of the chain
extern int32_t current_consecutive_rom_start; // start address of the current chain

// Console specific
void add_console_specific_game_json();

// CPU
void libRR_log_function_call(uint32_t current_pc, uint32_t target_pc, uint32_t stack_pointer=0);
void libRR_log_interrupt_call(uint32_t current_pc, uint32_t target_pc);
void libRR_log_long_jump(uint32_t current_pc, uint32_t jump_target, const char* type);
void libRR_log_branch(uint32_t current_pc, uint32_t target_pc);
void libRR_log_return_statement(uint32_t current_pc, uint32_t return_target, uint32_t stack_pointer=0);

extern bool libRR_full_function_log;
extern bool libRR_isDelaySlot;
extern uint32_t libRR_delay_slot_pc;
void libRR_log_trace_str(string message);

// Input 
extern void libRR_resave_button_state_to_file(string filename = "button_log.bin", int max_number=-1, json changes = json::parse("{}"));
extern void libRR_save_button_state_to_file(string filename  = "button_log.bin");
extern void libRR_read_button_state_from_file(string filename  = "button_log.bin", int start_frame=0);

// Save states
string libRR_load_save_state(int frame);

// Bank Switching
extern uint16_t libRR_current_bank;
extern uint32_t libRR_bank_size;
extern uint32_t libRR_bank_0_max_addr; // if address is smaller than this then it will be in bank 0 by default
extern uint32_t libRR_bank_1_max_addr; // if address is greater than this then its not ROM code
extern bool libRR_bank_switching_available;

// Scripting support
void libRR_run_script(string code);
int libRR_get_current_lba();
void* libRR_get_current_buffer();
void libRR_memset(int startOffset, int length, uint8_t value );
void libRR_cd_set_able_override(bool enable);
void libRR_replace_lba_buffer(int lba);

// CD
void libRR_override_cd_lba(uint8_t *buf, int32_t lba, int mode);
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
void libRR_video_cb(const void *fb, unsigned int width, unsigned int height, unsigned int pitch);


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
string libRR_parse_message_from_web(json message);

// Source sxport
void libRR_export_template_files(string template_directory_name);
extern string libRR_export_assembly_extention;

// Override with Console specific
void libRR_setup_console_details(retro_environment_t environ_cb);
void libRR_handle_emulator_close();
void libRR_export_all_files();
string get_builtin_function_name(const unsigned int offset, int bank);

// extern "C" {
  void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
// }

#ifdef __cplusplus
}
#endif

#endif