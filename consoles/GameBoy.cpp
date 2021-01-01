#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/mustache.hpp"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
using namespace kainjow::mustache;

extern "C" {

  json allLabels = {};

  const char* libRR_console = "GameBoy";

  // SameBoy doesn't have this defined so:
  char retro_base_directory[4096];

  // both libRR_retromap and libRR_mmap are used to setup memory regions
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
  uint32_t libRR_delay_slot_pc = 0;
  bool libRR_isDelaySlot = false;

  // Bank switching
  uint32_t libRR_bank_size = 0x4000; // 16KB
  uint16_t libRR_current_bank_slot_0 = 0; // 
  uint16_t libRR_current_bank_slot_1 = 1; // one by default for games that don't have mbc
  uint16_t libRR_current_bank_slot_2 = 1; // we don't have slot 2 available for switching
  uint32_t libRR_slot_0_max_addr = libRR_bank_size;
  uint32_t libRR_slot_1_max_addr = 0x7fff;
  uint32_t libRR_slot_2_max_addr = 0x7fff; // Gb doesn't have a slot 2 for banking
  bool libRR_bank_switching_available = true;

  bool should_stop_writing_asm(int offset, int i, string bank_number);


// libRR_set_retro_memmap is be called to setup memory ranges in a function such as retro_set_memory_maps
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

  int get_current_bank_number_for_address(uint32_t addr) {
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
    return 0; // TODO: replace with -1 when we can handle it
  }

  // 
  // Gameboy Z80 Start
  // 
  string libRR_contant_replace(int16_t da8) {
      // TODO: read these from JSON config instead
      switch(da8) {
          case 0x0091:
              return "LY_VBLANK";
          case 0xFF0F:
              return "rIF";
          case (int16_t)0xFF00:
              return "rJOYP";
      }
      if (da8 == (int16_t)0xFF0F) { 
          return "rIF";
      }
      if (da8 == (int16_t)0xFF40) { 
          return "rLCDC";
      }
      if (da8 == (int16_t)0xffff) { 
          return "rIE"; // Interrupt enable
      }
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

  string write_section_header(string offset_str, string bank_number, string section_name) {
    // first get the offset as int before appending "$"
    int32_t offset = hex_to_int(offset_str);
    string contents = "";
    offset_str = "$"+ offset_str;
    if (offset< 0x4000 || bank_number == "0000") {
      contents += "SECTION \"" + section_name + "\",ROM0["+offset_str+"]\n";
    } 
    else {
      contents += "SECTION \"" + section_name + "\",ROMX["+offset_str+"],BANK[$"+bank_number+"]\n";
    }
    return contents;
  }

// RGBDS crashes when a label is used but not defined
// there are cases where data is between parts of a function
// so we need a way to know labels that have not been generated
  void get_all_assembly_labels() {

    for (auto& bank : libRR_disassembly.items()) {
      string current_bank = bank.key();
        for (auto& instruction : libRR_disassembly[current_bank].items()) {
          if (libRR_disassembly[current_bank][instruction.key()].contains("label_name")) {
            string label_name = libRR_disassembly[current_bank][instruction.key()]["label_name"];
            json data = {};
            data["written"] = false;
            data["bank"] = bank.key();
            data["offset"] = instruction.key();
            allLabels[label_name] = data;
          }
        }
    }
  }

  string write_bank_header_comment(string bank) {
    string contents = "\n\n;;;;;;;;;;;\n; Bank:";
    contents += bank;
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
      bank = libRR_current_bank_slot_1;
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
    bool read_first_byte = false;
    for (auto& byteValue : dataRange.items()) {

      // Check if this overlaps with the start of another block (only after first byte)
      if (read_first_byte && libRR_consecutive_rom_reads[bank_number].contains(byteValue.key())) {
        break;
      }
      read_first_byte = true;

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
        // cout << "Address has been defined as another jump:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of data
      if (libRR_consecutive_rom_reads[bank_number].contains(n2hexstr(i))) {
        // cout << "Address has been defined as data:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of function
      if (libRR_called_functions[bank_number].contains(n2hexstr(i))) {
        // cout << "Address has been defined as a function:" << bank_number << "::" << n2hexstr(i) << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }

      if (bank_number=="0000" && i>= libRR_slot_0_max_addr) {
        cout << "Reached Max Bank 0 address \n";
        return true;
      }
      if (i>= libRR_slot_1_max_addr) {
        cout << "Reached Max Bank address \n";
        return true;
      }

      return false;
  }

  string write_asm_until_null(string bank_number, string offset_str, bool is_function) {
    string contents = "";
    int offset = hex_to_int(offset_str);
    int return_offset_from_start = 1100; // this is just the max, will most likely stop before this
    
    // First Check if this address is the starting address of function
      if (!is_function && libRR_called_functions[bank_number].contains(n2hexstr(offset))) {
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
        string label_name = (string)libRR_disassembly[bank_number][n2hexstr(i)]["label_name"];
        if ((bool)allLabels[label_name]["written"]) {
          contents += ";Already written this label\n";
          return contents;
        }
        contents +=  label_name + ":\n";
        allLabels[label_name]["written"] = true;
      }
      contents += "\t";
      for (auto& el : libRR_disassembly[bank_number][n2hexstr(i)].items()) {
        // std::cout << el.key() << " : " << el.value() << "\n";
        if (el.key() == "label_name" || el.key() == "meta") {
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
        // break; // break in the first non executed instruction
        // std::cout << "Not written: bank:" << bank_number << " offset:" << n2hexstr(i) << " " << libRR_disassembly[bank_number][n2hexstr(i)].dump() << "\n";
      } else {
        // write offset anyway for debugging
        contents += n2hexstr(i);
      }
      contents +="\n";
      i+=instruction_length;
    }
    contents+="; Reached max number of instruction bytes\n\n";
    
    return contents;
  }

void get_all_unwritten_labels() {

    string output_file_path = libRR_export_directory + "unwritten_relative_jumps.asm";
    string contents = "; Contains Relative jumps that executed but not written in jumps or functions due to being interrupted by data in between the code\n";

    for (auto& label : allLabels.items()) {
      if (! (bool) label.value()["written"]) {
        contents += "\n\n; Unwritten relative jump:" + label.key() + "\n";
        int offset = hex_to_int(label.value()["offset"]);
        string section_name = "REL_JMP_";
        section_name += label.value()["bank"];
        section_name += "_";
        section_name += label.value()["offset"];

        if (offset > libRR_slot_1_max_addr) {
          cout << "offset > libRR_slot_1_max_addr" << section_name << "\n";
          continue;
        }
        contents += write_section_header(label.value()["offset"], label.value()["bank"], section_name);
        cout << "Unwritten Label:" << label.key() << " = " << label.value() << "\n";

        json callers = libRR_disassembly[(string)label.value()["bank"]][(string)label.value()["offset"]]["meta"]["label_callers"];
        contents += write_callers(callers);

        contents += write_asm_until_null(label.value()["bank"], label.value()["offset"], false);
      }
    }

    codeDataLogger::writeStringToFile(output_file_path, contents);
    cout << "Written: " << output_file_path << "\n";
  }

  void libRR_export_jump_data() {
    string output_file_path = libRR_export_directory + "jumps.asm";
    string contents = "; Contains Long Jump data\n";
    // contents +="INCLUDE \"./common/constants.asm\"\n";
    // libRR_long_jumps

    for (auto& bank : libRR_long_jumps.items()) {
      contents += write_bank_header_comment(bank.key());
      
      for (auto& dataSection : bank.value().items()) {
          contents += "\nSECTION \"JMP_" + bank.key() + "_" + dataSection.key();
          if (bank.key() == "0000") {
            contents += "\",ROM0[$"+dataSection.key()+"]\n";
        } else {
            contents += "\",ROMX[$"+dataSection.key()+"],BANK[$"+bank.key()+"]\n";
        }
        contents += write_callers(dataSection.value());
        contents += write_asm_until_null(bank.key(), dataSection.key(), false);
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

        if (bank.key() == "0000" && dataSection.key() == "000000B1") {
          // Skip this as it always overlaps with code
          // TODO: find out why
          cout << "Skip 0xB1\n";
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

  string get_function_name(string bank, string offset) {
    string function_name = "_"+bank+"_func_"+offset;
    return function_name;
  }

  string get_function_export_path(string offset, json func, string bank) {
    string export_path = "";
    if (func.contains("export_path") && ((string)func["export_path"]).length()>0) {
      export_path = func["export_path"];
    } else {
      // Use a default generated export path
      export_path = "/functions/" + get_function_name(bank, offset) + libRR_export_assembly_extention;
    }
    return export_path;
  }

  void libRR_export_function_data() {

    string main_asm_contents = "INCLUDE \"./common/constants.asm\"\n";

    // Loop through each bank
    for (auto& bank : libRR_called_functions.items()) {

      // Now loop over functions inside bank
      for (auto& func : bank.value().items()) {
        string bank_str = bank.key();
        string func_offset_str = func.key();
        int32_t func_offset = hex_to_int(func_offset_str);
        string export_path = get_function_export_path(func_offset_str, func.value(), bank_str);

        if (func_offset >= 0xff80) {
          main_asm_contents += "; Ignore HRAM for now\n\n";
          continue;
        }
        
        main_asm_contents+="INCLUDE \"."+export_path+"\"\n";

        // Now write the actual file
        string output_file_path = libRR_export_directory + export_path;
        // create any folder that needs to be created
        std::__fs::filesystem::create_directories(codeDataLogger::dirnameOf(output_file_path));
        string function_name = get_function_name(bank_str, func_offset_str);

        string contents = "";
        contents += write_section_header(func_offset_str, bank_str, function_name);
        contents += write_asm_until_null(bank_str, func_offset_str, true);
        codeDataLogger::writeStringToFile(output_file_path, contents);
        cout << "Written file to: " << output_file_path << "\n";

      }

      

    }

    main_asm_contents+="\nINCLUDE \"jumps.asm\"\n";
    main_asm_contents+="\nINCLUDE \"unwritten_relative_jumps.asm\"\n";

    // Generate main.asm file, which includes the other files
    codeDataLogger::writeStringToFile(libRR_export_directory+"main.asm", main_asm_contents);

  }

  void libRR_export_all_files() {
    printf("GameBoy: Export All files to Reversing Project, %s \n", libRR_export_directory.c_str());
    // Copy over common template files
    libRR_export_template_files("gameboy");
    get_all_assembly_labels();
    libRR_export_rom_data();
    libRR_export_jump_data();
    libRR_export_function_data();
    get_all_unwritten_labels();
    
  }


}

