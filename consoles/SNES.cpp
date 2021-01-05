#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
#include "../source_exporter/CommonSourceExport.h"

extern "C" {

  // SNES9x doesn't have this defined so:
  char retro_base_directory[4096];
  // The following are only needed for consoles with CDs
  string retro_cd_base_directory = "libRR_RememberToSetCDBase";
  string retro_cd_path = "libRR_RememberToSetCDPATH";
  string retro_cd_base_name = "libRR_RememberToBaseName";
  int libRR_total_banks = 2;

  const char* libRR_console = "SNES";

  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[0];
  int libRR_mmap_descriptors = 0;
  int libRR_emulated_hardware = 0; // only used for emulators that support multiple consoles e.g GameGear/MasterSystem

  // Delay slot variables
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank_slot_0 = 0;
  uint16_t libRR_current_bank_slot_1 = 0;
  uint16_t libRR_current_bank_slot_2 = 0;
  uint32_t libRR_slot_0_max_addr = 0xfffffe;
  uint32_t libRR_slot_1_max_addr = 0xfffffe;
  uint32_t libRR_slot_2_max_addr = 0xfffffe;
  bool libRR_bank_switching_available = true;

  uint32_t libRR_pc_lookahead = 0;

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n");
    // libRR_set_retro_memmap(environ_cb);
    libRR_finished_boot_rom = true; // TODO put this in proper place
  }

// libRR_set_retro_memmap should be called to setup memory ranges in a function such as retro_set_memory_maps
  void libRR_set_retro_memmap(retro_memory_descriptor* descs, int num_descriptors)
  {
    for (int i=0; i<=10; i++) {
        libRR_mmap[i] = descs[i];
    }
    libRR_retromap.descriptors = libRR_mmap;
    libRR_retromap.num_descriptors = num_descriptors;
  }

  int get_current_bank_number_for_address(uint32_t addr) {
    return libRR_current_bank_slot_0;
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

  string write_console_asm_header() {
    string contents = "";
    contents+= ";==============================================================\n";
    contents+= "; WLA-DX banking setup\n";
    contents+= ";==============================================================\n";
    contents+= ".memorymap\n";
    contents+= "slotsize $4000\n";
    contents+= "slot 0 $0000\n";
    contents+= "slot 1 $4000\n";
    contents+= "slot 2 $8000\n";
    contents+= "defaultslot 2\n";
    contents+= ".endme\n\n";

    contents+= ".rombankmap\n";
    contents+= "bankstotal ";
    contents+= to_string(libRR_total_banks);
    contents+= "\nbanksize $4000\n";
    contents+= "banks ";
    contents+= to_string(libRR_total_banks);
    contents+= "\n.endro;\n\n";
    // contents+= "; SDSC tag and GG rom header\n\n";
    // contents+= ".sdsctag 1.0, \"Hello libRR\", \"Version\", \"rr\"\n\n";
    return contents;
  }

  void libRR_export_all_files() {
    printf("SNES: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files(libRR_console);  
    get_all_assembly_labels();
    // libRR_export_jump_data();
    libRR_export_function_data();
  }

  string get_slot_for_address(int32_t offset) {
    return "0";
  }

}

