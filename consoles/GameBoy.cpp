#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/mustache.hpp"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
using namespace kainjow::mustache;

extern "C" {

  const char* libRR_console = "GameBoy";

  // SameBoy doesn't have this defined so:
  char retro_base_directory[4096];

  struct retro_memory_map libRR_retromap = { 0 };

  struct retro_memory_descriptor libRR_mmap[11] = { 0 };
  // int libRR_mmap_descriptors = 11;

  // SameBoy's libretro doesn't have these paths defined
  string retro_cd_base_directory = "libRR_RemeberToSetCDBase";
  string retro_cd_path = "libRR_RemeberToSetCDPATH";
  string retro_cd_base_name = "libRR_RemeberToBaseName";
  // char retro_system_directory[4096];
  // char retro_save_directory[4096];
  // char retro_game_path[4096];
  // end Sameboy libretro differences

  // libRR_pc_lookahead is used to subtract from the Program counter in order to get the actual location of the Return instruction
  // For saturn we remove 2 from the program counter, but this will vary per console
  // we only want to return 2 and not 4 because we want to include the delay slot instruction
  uint32_t libRR_pc_lookahead = 0;

  // GameBoy does not have delay slots, but need to be defined anyway
  uint32_t libRR_delay_slot_pc;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0x4000; // 16KB
  uint32_t libRR_current_bank = 0;
  bool libRR_bank_switching_available = true;


  void libRR_set_retro_memmap(retro_memory_descriptor* descs, int num_descriptors)
  {
    for (int i=0; i<=10; i++) {
        libRR_mmap[i] = descs[i];
    }
    libRR_retromap.descriptors = libRR_mmap;
    libRR_retromap.num_descriptors = num_descriptors;
  }

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n",0);
    // libRR_set_retro_memmap(environ_cb);
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

  string generate_asm_for_function(const unsigned int offset, cdl_labels function) {
    string contents = "; TODO: generate ASM\n";
    string bank_number = function.bank_number;
    string offset_str = "$"+ n2hexstr(function.bank_offset);
    if ((uint8_t)offset == (uint8_t)0x40) {
      return "; Ignore Vblank for now";
    }

    if (function.bank_offset< 0x4000 || bank_number == "0") {
      contents += "SECTION \"" + function.func_name + "\",ROM0["+offset_str+"]\n\n";
    } 
    else if (function.bank_offset >= 0xff80) {
      return "; Ignore HRAM for now";
    }
    else {
      contents += "SECTION \"" + function.func_name + "\",ROMX["+offset_str+"],BANK[$"+bank_number+"]\n\n";
    }

    contents += function.func_name + "::\n";
    contents += "\txor a\n";
    contents += "\tret\n";
    // TODO: get ASM for function
    return contents;
  }


  void libRR_export_all_files() {
    printf("GameBoy: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files("gameboy");

    string main_asm_contents = "";
    for (auto& it : functions) {
      printf("test\n");
      cout << "offset:" << it.first << " name: " << functions[it.first].func_name << "\n";
      string export_path = "";
      if (functions[it.first].export_path.length()>0) {
        export_path = functions[it.first].export_path;
      } else {
        // Use a default generated export path
        export_path = "/functions/" + functions[it.first].func_name + libRR_export_assembly_extention;
      }
      main_asm_contents+="INCLUDE \"."+export_path+"\"\n";

      string output_file_path = libRR_export_directory + export_path;
      // create any folder that needs to be created
      std::__fs::filesystem::create_directories(codeDataLogger::dirnameOf(output_file_path));


      string contents = generate_asm_for_function(it.first, it.second);
      codeDataLogger::writeStringToFile(output_file_path, contents);
      cout << "Written file to: " << output_file_path << "\n";
      

      // if (it.second.func_offset == state["func_offset"]) {
      //   printf("Found function to update %s \n", state["func_offset"].dump().c_str());
      //   functions[it.first].func_name = state["func_name"];
      //   functions[it.first].export_path = state["export_path"];
      //   // functions[it.first].additional = message_json["state"]["additional"];
      //   break;
      // }
  }

    // Generate main.asm file, which includes the other files
    codeDataLogger::writeStringToFile(libRR_export_directory+"main.asm", main_asm_contents);
    
  }


}

