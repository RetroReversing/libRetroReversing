#include <queue>
#include "../civetweb/include/civetweb.h"

// Common Variables
int l_CurrentFrame;

extern "C" {
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

  void write_rom_mapping() {
    // save_cdl_files();
    // printf("ROM_PARAMS.headername: %s \n", rom_name.c_str());
    // string filename = "./configs/";
    // filename+=rom_name;
    // filename += ".config.yaml";
    // ofstream file(filename, std::ios_base::binary);
    // file << "# ROM splitter configuration file\n";
    // file << "name: \"";
    // file << rom_name;
    // file << "\"\n";
    // file <<"\n# base filename used for outputs - (please, no spaces)\n";
    // file <<"basename: \"";
    // file << rom_name;
    // file << "\"\n";
    // file <<"ranges:\n";
    // file <<"  # start,  end,      type,     label\n";
    // // file <<"  - [0x000000, 0x000040, \"header\", \"header\"]\n";   

    // //
    // // Write out 
    // //
    // for (auto& it : dmas) {
    //     auto t = it.second;
    //     if (it.first ==0 || t.dram_start == 0) continue;
    //     // file << create_n64_split_regions(t) << "\n";
    // }

    // file <<"# Labels for functions or data memory addresses\n";
    // file <<"# All label addresses are RAM addresses\n";
    // file <<"# Order does not matter\n";
    // file <<"labels:\n";
    // uint32_t entryPoint = 0x00; //ROM_HEADER.PC; // (int8_t*)(void*)&ROM_HEADER.PC;
    // file << "   - [0x" << std::hex << __builtin_bswap32(entryPoint)+0 <<", \"EntryPoint\"]\n";
    // for (auto& it : labels) {
    //     auto t = it.second;
    //     if (strcmp(t.func_offset.c_str(), "") == 0) continue;
    //     file << "   - [0x" << t.func_offset <<", \"" <<  t.func_name << "\"]\n";
    // }

  }

}