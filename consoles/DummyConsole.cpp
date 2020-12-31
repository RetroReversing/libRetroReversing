#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"

extern "C" {

  json allLabels = {};

  const char* libRR_console = "Dummy";

  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[0];
  int libRR_mmap_descriptors = 0;
  int libRR_emulated_hardware = 0; // only used for emulators that support multiple consoles e.g GameGear/MasterSystem

  // Delay slot variables
  uint32_t libRR_delay_slot_pc;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank_slot_0 = 1;
  uint16_t libRR_current_bank_slot_1 = 2;
  uint16_t libRR_current_bank_slot_2 = 3;
  uint32_t libRR_slot_0_max_addr = 0x4000;
  uint32_t libRR_slot_1_max_addr = 0x7fff;
  uint32_t libRR_slot_2_max_addr = 0x7fff;
  bool libRR_bank_switching_available = false;

  uint32_t libRR_pc_lookahead = 0;

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n");
    // libRR_set_retro_memmap(environ_cb);
  }

  void libRR_set_retro_memmap(retro_memory_descriptor* descs, int num_descriptors)
  {
    for (int i=0; i<=10; i++) {
        libRR_mmap[i] = descs[i];
    }
    libRR_retromap.descriptors = libRR_mmap;
    libRR_retromap.num_descriptors = num_descriptors;
  }

  string libRR_contant_replace(int16_t da8) {
    return "$"+n2hexstr(da8);
  }

  void console_log_jump_return(int take_jump, uint32_t jump_target, uint32_t pc, uint32_t ra, int64_t* registers, void* r4300) {
    printf("%d\n",1);
  }

  void main_state_load(const char *filename)
  {
    // TODO: actually load the state here
  }

  void main_state_save(int format, const char *filename)
  {
    // TODO: actually load the state here
  }

  void add_console_specific_game_json() {
    // TODO: Add special value to game_json e.g:
    // game_json["VDP2"]["TVMD"]["DisplayOn"] = VDP2::DisplayOn;
  }

  void write_rom_mapping() {
  }

  void libRR_export_all_files() {
    printf("Dummy: Export All files to Reversing Project, depends on which core we are using");
  }

}

