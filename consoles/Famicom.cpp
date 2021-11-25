#include <queue>
#include <iostream>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
#include "../source_exporter/CommonSourceExport.h"

  struct ines_header_t {
    uint8_t signature [4];
    uint8_t prg_count; // number of 16K PRG banks
    uint8_t chr_count; // number of 8K CHR banks
    uint8_t flags;     // MMMM FTBV Mapper low, Four-screen, Trainer, Battery, V mirror
    uint8_t flags2;    // MMMM --XX Mapper high 4 bits
    uint8_t zero [8];  // if zero [7] is non-zero, treat flags2 as zero
  };
  ines_header_t libRR_iNes = {};


extern "C" {

  void libRR_direct_serialize(void *data, size_t size) {
    // TODO: implement this as a save function that is called internally just to save
    retro_serialize(data, size);
  }

  bool libRR_direct_unserialize(const void *data, size_t size) {
    // TODO: implement this properly
    retro_unserialize(data, size);
  }

  // GenesisPlusGX doesn't have this defined so:
  char retro_base_directory[4096];
  // The following will need to be added for Sega Mega CD support
  string retro_cd_base_directory = "libRR_RememberToSetCDBase";
  string retro_cd_path = "libRR_RememberToSetCDPATH";
  string retro_cd_base_name = "libRR_RememberToBaseName";
  string retro_save_directory = "libRR_RememberToBaseName";

  const char* libRR_console = "NES";
  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[0];
  int libRR_mmap_descriptors = 0;
  int libRR_emulated_hardware = 0; // only used for emulators that support multiple consoles e.g GameGear/MasterSystem

  // Delay slot variables
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank_slot_0 = 1;
  uint16_t libRR_current_bank_slot_1 = 2;
  uint16_t libRR_current_bank_slot_2 = 3;
  uint32_t libRR_slot_0_max_addr = 0x8000*2;
  uint32_t libRR_slot_1_max_addr = 0x8000*2;
  uint32_t libRR_slot_2_max_addr = 0x8000*2;
  bool libRR_bank_switching_available = false;

  uint32_t libRR_pc_lookahead = 0;

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n");
    // libRR_set_retro_memmap(environ_cb);
    libRR_finished_boot_rom = true;
    // For some reson if these aren't defined here then quickNES will segfault due to corruption of the default values
    libRR_org_directive = ".ORGA";
    libRR_bank_directive = ".BANK $";
    libRR_slot_directive = "SLOT";
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
    return 0;
  }

  string get_slot_for_address(int32_t offset) {
    return "0";
  }

  void write_ines_header_file() {
    ofstream myFile (libRR_export_directory + "iNES.bin", ios::out | ios::binary);
    myFile.write ((char*)&libRR_iNes, 16);
    myFile.close();

  }

  string write_console_asm_header() {
    string contents = "";
    contents+= ";==============================================================\n";
    contents+= "; WLA-DX banking setup\n";
    contents+= ";==============================================================\n";
    // TODO: get the correct values from the iNES header
    // e.g Mario has 2x8KB code banks and 1x8KB Graphics bank
    contents+= ".memorymap\n";
    contents+= "DEFAULTSLOT 0 \n";
    contents+= "slotsize $8000\n";
    contents+= "SLOT 0 $8000 ; NES Starts at 0x8000 \n";
    contents+= ";slotsize $2000 ; CHR-ROM slot size\n";
    contents+= ";SLOT 1 $0000 ; CHR-ROM slot 1 starts at 0x00\n";
    contents+= ".endme\n\n";

    contents+= ".rombankmap\n";
    contents+= "bankstotal ";
    contents+= to_string(libRR_total_banks+1); // add one for the CHR bank
    contents+= "\nbanksize $8000\n";
    contents+= "banks ";
    contents+= to_string(libRR_total_banks);
    contents+= "\nbanksize $2000\n";
    contents+= "banks 1 ; CHR-ROM";
    contents+= "\n.endro;\n\n";

    write_ines_header_file();

    return contents;
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
    printf("NES: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files(libRR_console);
    get_all_assembly_labels();
    libRR_export_rom_data();
    libRR_export_jump_data();
    libRR_export_function_data();
    get_all_unwritten_labels();
  }

}

