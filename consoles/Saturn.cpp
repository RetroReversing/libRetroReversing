#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/CDL.hpp"

// Common Variables

namespace VDP2
{
  extern uint16_t* RawRegs;
  // TVMD Registers - TV Mode
  extern bool BorderMode;
  extern bool DisplayOn;
  extern uint8_t InterlaceMode;
  extern uint8_t HRes, VRes;

  // Exten registers - Extension?
  extern bool ExSyncEnable;
  extern bool ExLatchEnable;
  extern bool ExBGEnable;
  extern bool DispAreaSelect;

  extern bool VRAMSize;

  extern uint16_t RAMCTL_Raw;
  extern uint8_t CRAM_Mode;
}

extern "C" {
  void libRR_log_branch(uint32_t current_pc, uint32_t target_pc) {

  }
  const char* libRR_console = "Saturn";

  // libRR_pc_lookahead is used to subtract from the Program counter in order to get the actual location of the Return instruction
  // For saturn we remove 2 from the program counter, but this will vary per console
  // we only want to return 2 and not 4 because we want to include the delay slot instruction
  uint32_t libRR_pc_lookahead = 2;

  // Delay slot variables
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  struct retro_memory_map libRR_retromap = { 0 };

  // Bank switching
  uint32_t libRR_bank_size = 0;
  uint16_t libRR_current_bank_slot_0 = 1;
  uint16_t libRR_current_bank_slot_1 = 2;
  uint16_t libRR_current_bank_slot_2 = 3;
  uint32_t libRR_slot_0_max_addr = 0x4000;
  uint32_t libRR_slot_1_max_addr = 0x7fff;
  uint32_t libRR_slot_2_max_addr = 0x7fff;
  bool libRR_bank_switching_available = false;
  
  void console_log_jump_return(int take_jump, uint32_t jump_target, uint32_t pc, uint32_t ra, int64_t* registers, void* r4300) {
    printf("%d\n",1);
  }

  int get_current_bank_number_for_address(uint32_t addr) {
    return 0;
  }

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n");
    // libRR_set_retro_memmap(environ_cb);
  }

  void write_rom_mapping() {

  }

  void main_state_save(int format, const char *filename)
  {
    // TODO: actually load the state here
  }

  void main_state_load(const char *filename)
  {
    // TODO: actually load the state here
  }

  void add_console_specific_game_json() {
    // more info on VDP2 regs: https://segaretro.org/images/8/89/TUTORIAL.pdf
    // game_json["VDP2"]["RawRegs"] = printBytesToDecimalJSArray((uint8_t*)VDP2::RawRegs, 0x100);
    
    // TVMD - TV Mode registers
    // game_json["VDP2"]["TVMD"]["DisplayOn"] = VDP2::DisplayOn;
    // game_json["VDP2"]["TVMD"]["BorderMode"] = VDP2::BorderMode;
    // game_json["VDP2"]["TVMD"]["InterlaceMode"] = VDP2::InterlaceMode;
    // game_json["VDP2"]["TVMD"]["HRes"] = VDP2::HRes;
    // game_json["VDP2"]["TVMD"]["VRes"] = VDP2::VRes;

    // // EXTEN - 
    // game_json["VDP2"]["EXTEN"]["ExSyncEnable"] = VDP2::ExSyncEnable;
    // game_json["VDP2"]["EXTEN"]["ExLatchEnable"] = VDP2::ExLatchEnable;
    // game_json["VDP2"]["EXTEN"]["ExBGEnable"] = VDP2::ExBGEnable;
    // game_json["VDP2"]["EXTEN"]["DispAreaSelect"] = VDP2::DispAreaSelect;

    // game_json["VDP2"]["VRAMSize"] = VDP2::VRAMSize;
    // game_json["VDP2"]["RAMCTL_Raw"] = VDP2::RAMCTL_Raw;
    // game_json["VDP2"]["CRAM_Mode"] = VDP2::CRAM_Mode;
    // Documentation only needs to be added once
    game_json["Documentation"]["Memory"]["VDP2CRAM"] = "Colour RAM for VDP2 - basically the palette for the background layer";
  }

  void libRR_export_all_files() {
    printf("Saturn: Export All files to Reversing Project, depends on which core we are using");
  }

  string get_slot_for_address(int32_t offset) {
    return "0";
  }

  string write_console_asm_header() {
    string contents = "";
    return contents;
  }

}