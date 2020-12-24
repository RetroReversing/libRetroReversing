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
  uint16_t libRR_current_bank = 0;
  uint32_t libRR_bank_0_max_addr = libRR_bank_size;
  bool libRR_bank_switching_available = true;

  bool should_stop_writing_asm(int offset, int i, string bank_number);


  void libRR_set_retro_memmap(retro_memory_descriptor* descs, int num_descriptors)
  {
    for (int i=0; i<=10; i++) {
        libRR_mmap[i] = descs[i];
    }
    libRR_retromap.descriptors = libRR_mmap;
    libRR_retromap.num_descriptors = num_descriptors;
  }

  void libRR_setup_console_details(retro_environment_t environ_cb) {
    //printf("TODO: Setup setting such as libRR_define_console_memory_region for this console\n",0);
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
    // Previous verion only got until last executed RET:
    // int return_offset_from_start = function.return_offset_from_start;
    int return_offset_from_start = 190;

    contents += "; end:" + n2hexstr(return_offset_from_start) + "\n";

    // if ((uint8_t)offset == (uint8_t)0x40) {
    //   return "; Ignore Vblank for now\n\n";
    // }

    if (function.bank_offset< 0x4000 || bank_number == "0000") {
      contents += "SECTION \"" + function.func_name + "\",ROM0["+offset_str+"]\n\n";
    } 
    else if (function.bank_offset >= 0xff80) {
      return "; Ignore HRAM for now\n\n";
    }
    else {
      contents += "SECTION \"" + function.func_name + "\",ROMX["+offset_str+"],BANK[$"+bank_number+"]\n\n";
    }

    contents += function.func_name + "::\n";

    if (return_offset_from_start > 200) {
      contents += "; return offset too large:" + n2hexstr(return_offset_from_start) + "\n\n";
      return contents;
    }

    // loop through disassembly
    for (int i=offset; i<=offset+return_offset_from_start;) {
      int instruction_length = 1;
      bool has_written_line = false;


      if (should_stop_writing_asm(offset, i, bank_number)) {
        break;
      }

      if (libRR_disassembly[bank_number][n2hexstr(i)].contains("label_name")) {
        contents += (string)libRR_disassembly[bank_number][n2hexstr(i)]["label_name"] + ":\n";
      }
      contents += "\t";
      for (auto& el : libRR_disassembly[bank_number][n2hexstr(i)].items()) {
        // std::cout << el.key() << " : " << el.value() << "\n";
        if (el.key() == "label_name") {
          continue;
        } else {
          contents += el.key() + " ;";
          instruction_length = el.value()["bytes_length"];
        }
        has_written_line = true;
      }
      if (!has_written_line) {
        contents += "nop ; not executed offset: ";
        contents += n2hexstr(i);
        // std::cout << "Not written: bank:" << bank_number << " offset:" << n2hexstr(i) << " " << libRR_disassembly[bank_number][n2hexstr(i)].dump() << "\n";
      } else {
        // write offset anyway for debugging
        contents += n2hexstr(i);
      }
      contents +="\n";
      i+=instruction_length;
    }
    contents += "\n";

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
      bank = libRR_current_bank;
    }

    libRR_log_memory_read(bank, offset, type, byte_size, bytes);
  }

  string write_callers(json callers) {
    string contents = "";
    for (auto& byteValue : callers.items()) {
      contents += "; Called by: ";
      contents += byteValue.key();
      contents += "\n";
    }
    return contents;
  }

  string write_each_rom_byte(string bank_number, json dataRange) {
    string contents = "";
    for (auto& byteValue : dataRange.items()) {

      // Check if this overlaps with the start of another block
      if (libRR_consecutive_rom_reads[bank_number].contains(byteValue.key())) {
        break;
      }

      contents += "\tdb $";
      contents += byteValue.value();
      contents += " ; ";
      contents += byteValue.key();
      contents += "\n";
    }
    return contents;
  }

  bool should_stop_writing_asm(int offset, int i, string bank_number) {
    if (i == offset) {
      return false;
    }
    // Check if this address is the starting address of another jump definition
      if (libRR_long_jumps[bank_number].contains(n2hexstr(i))) {
        cout << "Address has been defined as another jump:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of data
      if (libRR_consecutive_rom_reads[bank_number].contains(n2hexstr(i))) {
        cout << "Address has been defined as data:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of function
      if (libRR_called_functions[bank_number].contains(n2hexstr(i))) {
        cout << "Address has been defined as a function:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      return false;
  }

  string write_asm_until_null(string bank_number, string offset_str) {
    string contents = "";
    int offset = hex_to_int(offset_str);
    int return_offset_from_start = 1000; // this is just the max, will most likely stop before this
    
    // First Check if this address is the starting address of function
      if (libRR_called_functions[bank_number].contains(n2hexstr(offset))) {
        cout << "Jump has already been defined as a function:" << bank_number << "::" << n2hexstr(offset) << "\n";
        if (libRR_disassembly[bank_number][n2hexstr(offset)].contains("label_name")) {
          contents += "; "+(string)libRR_disassembly[bank_number][n2hexstr(offset)]["label_name"] + " EQU $";
          contents += n2hexstr(offset);
        }
        contents += "; Address Also defined as function\n";
        return contents;
      }

    // loop through disassembly
    for (int i=offset; i<=offset+return_offset_from_start;) {
      int instruction_length = 1;
      bool has_written_line = false;

      

      if (should_stop_writing_asm(offset, i, bank_number)) {
        contents+=";stopped writing due to overlap with another section\n";
        return contents;
      }

      if (libRR_disassembly[bank_number][n2hexstr(i)].contains("label_name")) {
        contents += (string)libRR_disassembly[bank_number][n2hexstr(i)]["label_name"] + ":\n";
      }
      contents += "\t";
      for (auto& el : libRR_disassembly[bank_number][n2hexstr(i)].items()) {
        // std::cout << el.key() << " : " << el.value() << "\n";
        if (el.key() == "label_name") {
          continue;
        } else {
          contents += el.key() + " ;";
          instruction_length = el.value()["bytes_length"];
        }

        // TODO: check for RET, CALL or JUMP instructions and exist if so, startswith

        has_written_line = true;
      }
      if (!has_written_line) {
        contents += "nop ; not executed offset: ";
        contents += n2hexstr(i);
        // break; // break in the first non executed instruction
        // std::cout << "Not written: bank:" << bank_number << " offset:" << n2hexstr(i) << " " << libRR_disassembly[bank_number][n2hexstr(i)].dump() << "\n";
      } else {
        // write offset anyway for debugging
        contents += n2hexstr(i);
      }
      contents +="\n";
      i+=instruction_length;
    }
    contents+="; Reached max number of instruction bytes";
    
    return contents;
  }

  void libRR_export_jump_data() {
    string output_file_path = libRR_export_directory + "jumps.asm";
    string contents = "; Contains Long Jump data\n";
    // contents +="INCLUDE \"./common/constants.asm\"\n";
    // libRR_long_jumps

    for (auto& bank : libRR_long_jumps.items()) {
      contents += "\n\n;;;;;;;;;;;\n; Bank:";
      contents += bank.key();
      contents += "\n";
      for (auto& dataSection : bank.value().items()) {
          contents += "\nSECTION \"JMP_" + bank.key() + "_" + dataSection.key();
          if (bank.key() == "0000") {
            contents += "\",ROM0[$"+dataSection.key()+"]\n";
        } else {
            contents += "\",ROMX[$"+dataSection.key()+"],BANK[$"+bank.key()+"]\n";
        }
        contents += write_callers(dataSection.value());
        contents += write_asm_until_null(bank.key(), dataSection.key());
      }
    }

    codeDataLogger::writeStringToFile(output_file_path, contents);
    cout << "Written jumps.asm to: " << output_file_path << "\n";
  }

  void libRR_export_rom_data() {
    string output_file_path = libRR_export_directory + "data.asm";
    string contents = "; Contains ROM static data\n";

    // Loop through each bank
    
    for (auto& bank : libRR_consecutive_rom_reads.items()) {
      // std::cout << "libRR_consecutive_rom_reads:" << bank.key() << " : " << bank.value() << "\n";
      contents += "\n\n;;;;;;;;;;;\n; Bank:";
      contents += bank.key();
      contents += "\n";
      for (auto& dataSection : bank.value().items()) {
        // std::cout << "dataSection: " << bank.key() << "::" << dataSection.key() << " : " << dataSection.value() << "\n";
        if (dataSection.value()["length"].is_null()) {
          // TODO: Need to fix these null lengths
          continue;
        }
        contents += "\nSECTION \"DAT_" + bank.key() + "_" + dataSection.key();
        if (bank.key() == "0000") {
            contents += "\",ROM0[$"+dataSection.key()+"]\n";
        } else {
            contents += "\",ROMX[$"+dataSection.key()+"],BANK[$"+bank.key()+"]\n";
        }

        contents += write_each_rom_byte(bank.key(), dataSection.value()["value"]);
      }
    }

    codeDataLogger::writeStringToFile(output_file_path, contents);
    cout << "Written data.asm to: " << output_file_path << "\n";
  }

  void libRR_export_all_files() {
    printf("GameBoy: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files("gameboy");
    libRR_export_rom_data();
    libRR_export_jump_data();

    string main_asm_contents = "INCLUDE \"./common/constants.asm\"\n";
    for (auto& it : functions) {
      // cout << "offset:" << it.first << " name: " << functions[it.first].func_name << "\n";
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
      

  }
    main_asm_contents+="\nINCLUDE \"jumps.asm\"\n";

    // Generate main.asm file, which includes the other files
    codeDataLogger::writeStringToFile(libRR_export_directory+"main.asm", main_asm_contents);
    
  }


}

