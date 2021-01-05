#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
#include "../source_exporter/CommonSourceExport.h"


extern "C" {

  json allLabels = {};

  // This is used for most SEGA 8bit and 16bit consoles, such as Master System, Game Gear and Mega Drive
  const char* libRR_console = "GenesisPlusGX";
  int libRR_emulated_hardware = 0;
  int libRR_total_banks = 2;

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
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank_slot_0 = 0;
  uint16_t libRR_current_bank_slot_1 = 1;
  uint16_t libRR_current_bank_slot_2 = 2;

// Game Gear and SMS values
  uint32_t libRR_slot_0_max_addr = 0x4000;
  uint32_t libRR_slot_1_max_addr = 0x8000;
  uint32_t libRR_slot_2_max_addr = 0xbfff;
  bool libRR_bank_switching_available = true;

  uint32_t libRR_pc_lookahead = 0;

  #define SYSTEM_GG 0x40
  #define SYSTEM_SMS        0x20
  #define SYSTEM_SMS2       0x21
  void libRR_setup_console_details(retro_environment_t environ_cb) {
    // printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n",0);
    // libRR_set_retro_memmap(environ_cb);
    printf("libRR_setup_console_details hardware:%d\n", libRR_emulated_hardware);
    if (libRR_emulated_hardware == SYSTEM_GG) {
      libRR_console = "GameGear";
    } else if (libRR_emulated_hardware == SYSTEM_SMS || libRR_emulated_hardware == SYSTEM_SMS2) {
      libRR_console = "MasterSystem";
    }
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

  int get_current_bank_number_for_address(uint32_t addr) {
    if (addr <= 0x03ff) {
      // Unpaged rom for game gear and SMS
      return 0;
    }
    if (addr < libRR_slot_0_max_addr) {
        return libRR_current_bank_slot_0;
    }
    if (addr >= libRR_slot_0_max_addr && addr< libRR_slot_1_max_addr) {
        return libRR_current_bank_slot_1;
    } 
    if (addr>= libRR_slot_1_max_addr && addr < libRR_slot_2_max_addr) {
        // target is in slot 2
        return libRR_current_bank_slot_2;
    }
    return -1;
  }

  string get_slot_for_address(int32_t offset) {
    if (offset < 0x4000) {
      return "0";
    }
    if (offset < 0x8000) {
      return "1";
    }
    return "2";
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
    contents+= "; SDSC tag and GG rom header\n\n";
    contents+= ".sdsctag 1.0, \"Hello libRR\", \"Version\", \"rr\"\n\n";
    return contents;
  }

  // get_builtin_function_name - If the address matches a built in function then return the name
  string get_builtin_function_name(const unsigned int offset, int bank) {
    if (bank == 0 && (uint8_t)offset == (uint8_t)0x40) {
      return "vblank";
    }
    return "";
  }

  void libRR_gameboy_log_memory_read(int32_t offset, const char* type, uint8_t byte_size, char* bytes) {
    int8_t bank = 0;
    if (offset >= 0x0000FE00) {
      // OAM
      // I/O registers
      // HRAM
      // Interrupts enable
        return; 
    }
    if (offset >= 0x0000C000) {
      // Work ram banks 0 and 1
      return;
    }
    if (offset >= 0x00008000) {
      // VRAM
      return;
    }
    if (offset>= 0x00004000) {
      bank = libRR_current_bank_slot_0;
    }

    libRR_log_memory_read(bank, offset, type, byte_size, bytes);
  }

  uint32_t last_written_byte_addr = 0;
  string write_each_rom_byte(string bank_number, json dataRange) {
    string contents = "";
    bool read_first_byte = false;
    for (auto& byteValue : dataRange.items()) {

      uint32_t byte_address = hex_to_int(byteValue.key());

      if (byte_address <= last_written_byte_addr) {
        contents += "; Already written: " + n2hexstr(byte_address) +"\n";
        last_written_byte_addr = byte_address;
        continue;
      }

      // Check if this overlaps with the start of another block (only after first byte)
      // if (read_first_byte && libRR_consecutive_rom_reads[bank_number].contains(byteValue.key())) {
        // so its possible for data to be read inside a previous consecutive read and not use the full lenfth
        // so we should write a label for it
        // break;
      // }
      read_first_byte = true;

      contents += "\t.db $";
      contents += byteValue.value();
      contents += " ; ";
      contents += byteValue.key();
      contents += "\n";
      last_written_byte_addr = byte_address;
    }
    return contents;
  }

  void libRR_export_rom_data() {
    string output_file_path = libRR_export_directory + "data.asm";
    string contents = "; Contains ROM static data\n";
    contents+=write_console_asm_header();

    // Loop through each bank
    for (auto& bank : libRR_consecutive_rom_reads.items()) {
      last_written_byte_addr = 0; // reset at the start of each bank

      contents += "\n\n;;;;;;;;;;;\n; Bank:";
      contents += bank.key();
      contents += "\n";
      for (auto& dataSection : bank.value().items()) {
        if (dataSection.value()["length"].is_null()) {
          // TODO: Need to fix these null lengths
          continue;
        }

        string contents_of_rom_section = write_each_rom_byte(bank.key(), dataSection.value()["value"]);

        if (contents_of_rom_section == "") {
          continue;
        }

        contents += write_section_header(dataSection.key(), bank.key(), "DAT_"+ bank.key() + "_" + dataSection.key());
        contents += contents_of_rom_section;
      }
    }

    codeDataLogger::writeStringToFile(output_file_path, contents);
    cout << "Written data.asm to: " << output_file_path << "\n";
  }

  void libRR_export_all_files() {
    printf("GameGear: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files(libRR_console);
    get_all_assembly_labels();
    libRR_export_rom_data();
    libRR_export_jump_data();
    libRR_export_function_data();
    get_all_unwritten_labels();
    
  }

}

