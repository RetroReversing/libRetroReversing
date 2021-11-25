#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
#include "../source_exporter/CommonSourceExport.h"

extern "C" {

  void libRR_direct_serialize(void *data, size_t size) {
    // TODO: implement this as a save function that is called internally just to save
    retro_serialize(data, size);
  }

  bool libRR_direct_unserialize(const void *data, size_t size) {
    // TODO: implement this properly
    retro_unserialize(data, size);
  }

  // SNES9x doesn't have this defined so:
  char retro_base_directory[4096];
  // The following are only needed for consoles with CDs
  string retro_cd_base_directory = "libRR_RememberToSetCDBase";
  string retro_cd_path = "libRR_RememberToSetCDPATH";
  string retro_cd_base_name = "libRR_RememberToBaseName";

  const char* libRR_console = "SNES";

  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[0];
  int libRR_mmap_descriptors = 0;
  int libRR_emulated_hardware = 0; // only used for emulators that support multiple consoles e.g GameGear/MasterSystem

  // Delay slot variables
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0x8000;
  uint32_t libRR_bank_start = 0x8000;
  uint16_t libRR_current_bank_slot_0 = 0;
  uint16_t libRR_current_bank_slot_1 = 0;
  uint16_t libRR_current_bank_slot_2 = 0;
  uint32_t libRR_slot_0_max_addr = 0x8000*2;
  uint32_t libRR_slot_1_max_addr = 0x8000*2;
  uint32_t libRR_slot_2_max_addr = 0x8000*2;
  bool libRR_bank_switching_available = true;

  uint32_t libRR_pc_lookahead = 0;
  string libRR_rom_type = "LoROM";

  // variable specific to SNES core
    #include "../src/port.h"
    #include "../src/memmap.h"
  extern CMemory	Memory;

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n");
    // libRR_set_retro_memmap(environ_cb);
    libRR_finished_boot_rom = true; // TODO put this in proper place
    if (Memory.LoROM) {
      libRR_rom_type = "LOROM";
    } else if (Memory.HiROM) {
      libRR_rom_type = "HIROM";
    }
    else {
      libRR_rom_type = "UNKNOWN";
    }
    cout << "SNES ROM TYPE:" << libRR_rom_type << "\n\n";
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
    if (addr > 0xffff) {
      int bank = (addr & 0xff0000) >> 16;
      // cout << "get_current_bank_number_for_address: " << n2hexstr(addr) << " bank" << n2hexstr(bank) <<  "\n";
      return bank;
    //   return addr & 0xffffff;
    }
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
    // currently this is for LoRom
    contents+= ".memorymap\n";
    contents+= "slotsize $8000\n";
    contents+= "SLOT 0 $8000 \n";
    contents+= "DEFAULTSLOT 0 ; only 1 slot in SNES\n";
    contents+= ".endme\n\n";

    contents+= ".rombankmap\n";
    contents+= "bankstotal ";
    contents+= to_string(libRR_total_banks);
    contents+= "\nbanksize $8000\n";
    contents+= "banks ";
    contents+= to_string(libRR_total_banks);
    contents+= "\n.endro;\n\n";

    contents+= ".SNESHEADER\n";
    contents+= "  ID \"SNES\"                     ; 1-4 letter string\n";
    contents+= " \n";
    contents+= "  NAME \"SNES Program Name    \"  ; Program Title - can't be over 21 bytes,\n";
    contents+= " \n";
    contents+= "  SLOWROM\n";
    contents+= libRR_rom_type+" \n";
    contents+= " \n";
    // TODO: the following are setup for Mario but need to be found dynamically
    contents+= "  CARTRIDGETYPE $002            ; $00 = ROM only, see WLA documentation for others\n";
    contents+= "  ROMSIZE $09                   ; 0x7FD7 $08 = 2 Mbits,  see WLA doc for more..\n";
    contents+= "  SRAMSIZE $01                  ; No SRAM         see WLA doc for more..\n";
    contents+= "  COUNTRY $01                   ; $01 = U.S.  $00 = Japan, that's all I know\n";
    contents+= "  LICENSEECODE $00              ; Just use $00\n";
    contents+= "  VERSION $00                   ; $00 = 1.00, $01 = 1.01, etc.\n";
    contents+= ".ENDSNES\n";

    contents+= ".SNESNATIVEVECTOR               ; Define Native Mode interrupt vector table\n";
    contents+= "  COP $82C3 ; ROM location: 0x7FE4\n";
    contents+= "  BRK $FFFF ;0x7FE6\n";
    contents+= "  ABORT $82C3 ;0x7FE8\n";
    contents+= "  NMI $816A ;0x7FEA\n";
    contents+= "  UNUSED $0000 ;0x7FEA\n";
    contents+= "  IRQ $8374 ;0x7FEE\n";
    contents+= ".ENDNATIVEVECTOR\n";
    contents+= " \n";
    contents+= ".SNESEMUVECTOR                  ; Define Emulation Mode interrupt vector table\n";
    contents+= "  COP $FFFF ; ROM location: $7FF4 \n";
    contents+= "  ABORT $82C3 ; $7FF8\n";
    contents+= "  NMI $82C3 ; $7FFA\n";
    contents+= "  RESET $8000 ; $7FFC\n";
    contents+= "  IRQBRK $82C3 ; $7FFE\n";
    contents+= ".ENDEMUVECTOR\n";

    return contents;
  }

  void libRR_export_all_files() {
    printf("SNES: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // libRR_slot_directive = ";no slots";
    // Copy over common template files
    libRR_export_template_files(libRR_console);  
    get_all_assembly_labels();
    libRR_export_rom_data();
    libRR_export_jump_data();
    libRR_export_function_data();
    get_all_unwritten_labels();
  }

  string get_slot_for_address(int32_t offset) {
    return "0";
  }

}

