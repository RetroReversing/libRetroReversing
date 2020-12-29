#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"

extern "C" {
  // This is used for most SEGA 8bit and 16bit consoles, such as Master System, Game Gear and Mega Drive
  const char* libRR_console = "GenesisPlusGX";
  int libRR_emulated_hardware = 0;

  // GenesisPlusGX doesn't have this defined so:
  char retro_base_directory[4096];
  // The following will need to be added for Sega Mega CD support
  string retro_cd_base_directory = "libRR_RememberToSetCDBase";
  string retro_cd_path = "libRR_RememberToSetCDPATH";
  string retro_cd_base_name = "libRR_RememberToBaseName";

  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[0];
  int libRR_mmap_descriptors = 0;

  // Delay slot variables
  uint32_t libRR_delay_slot_pc;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank = 1;
  uint32_t libRR_bank_0_max_addr = 0x4000;
  uint32_t libRR_bank_1_max_addr = 0x7fff;
  bool libRR_bank_switching_available = false;

  uint32_t libRR_pc_lookahead = 0;

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    // printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n",0);
    // libRR_set_retro_memmap(environ_cb);
    printf("libRR_setup_console_details hardware:%d\n", libRR_emulated_hardware);
    libRR_finished_boot_rom = true;
  }

  void libRR_set_retro_memmap(retro_memory_descriptor* descs, int num_descriptors)
  {
    for (int i=0; i<=10; i++) {
        libRR_mmap[i] = descs[i];
    }
    libRR_retromap.descriptors = libRR_mmap;
    libRR_retromap.num_descriptors = num_descriptors;
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

