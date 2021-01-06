#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/mustache.hpp"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
#include "./CommonSourceExport.h"
using namespace kainjow::mustache;

string libRR_export_assembly_extention = ".s";
json allLabels = {};
string include_directive = ".INCLUDE";

bool libRR_replace_string(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void libRR_export_template_files(string template_directory_name) {
    json files_json;
    string export_template_directory = "./libRetroReversing/export_templates/"+template_directory_name+"/";
    readJsonToObject(export_template_directory+"files.json", files_json);
    cout << "Exporter Name is: " << files_json["name"] << "\n";
    libRR_export_assembly_extention = (string)files_json["asm_extenstion"];

    // Create required directories
    json dirs = files_json["dirs"];
    for (json::iterator it = dirs.begin(); it != dirs.end(); ++it) {
      string path_to_create = libRR_export_directory + (string)*it;
      cout << "Creating directory: " << path_to_create << "\n";
      fs::create_directories(path_to_create);
    }

    // Copy required files
    json j = files_json["files"];
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
      string current = *it;
      string contents = codeDataLogger::readFileToString(export_template_directory + current);

      mustache tmpl{contents};

      string output_file_path = libRR_export_directory + current;
      // create any folder that needs to be created
      fs::create_directories(codeDataLogger::dirnameOf(output_file_path));

      // Now save the file to the project directory
      codeDataLogger::writeStringToFile(output_file_path, 
        tmpl.render({"GAME_NAME", libRR_game_name}) );
      cout << "Written file to: " << output_file_path << "\n";

    }
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

string write_callers(json callers) {
  string contents = "";
  for (auto& byteValue : callers.items()) {
    contents += "; Called by: ";
    contents += byteValue.key();
    contents += "\n";
  }
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

    for (auto& bank : libRR_long_jumps.items()) {
      contents += write_bank_header_comment(bank.key());
      
      for (auto& dataSection : bank.value().items()) {
        contents += write_section_header(dataSection.key(), bank.key(), "JMP_"+ bank.key() + "_" + dataSection.key());
        contents += write_callers(dataSection.value());
        contents += write_asm_until_null(bank.key(), dataSection.key(), false);
      }
    }

    codeDataLogger::writeStringToFile(output_file_path, contents);
    cout << "Written jumps.asm to: " << output_file_path << "\n";
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

string get_function_name(string bank, string offset) {
    string function_name = "_"+bank+"_func_"+offset;
    return function_name;
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
        contents+=";stopped writing due to overlap with another section "+ n2hexstr(offset) +"\n";
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

bool should_stop_writing_asm(int start_offset, uint32_t i, string bank_number) {
    if (i == start_offset) {
      return false;
    }

    string current_address_str = n2hexstr(i);
    // Check if this address is the starting address of another jump definition
      if (libRR_long_jumps[bank_number].contains(current_address_str)) {
        cout << "Address has been defined as another jump:" << bank_number << "::" << current_address_str << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of data
      if (libRR_consecutive_rom_reads[bank_number].contains(current_address_str)) {
        cout << "Address has been defined as data:" << bank_number << "::" << current_address_str << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }
      // Check if this address is the starting address of function
      if (libRR_called_functions[bank_number].contains(current_address_str)) {
        cout << "Address has been defined as a function:" << bank_number << "::" << current_address_str << "\n";
        // contents += "; Address defined as another jump\n";
        return true;
      }

      if (bank_number=="0000" && i>= libRR_slot_0_max_addr) {
        cout << current_address_str << " Reached Max Bank 0 address \n";
        return true;
      }
      if ((uint32_t)i > libRR_slot_2_max_addr) {
        cout << current_address_str << "Reached Max Bank address \n";
        return true;
      }

      return false;
  }

string write_bank_header_comment(string bank) {
    string contents = "\n\n;;;;;;;;;;;\n; Bank:";
    contents += bank;
    contents += "\n";
    return contents;
  }

string libRR_org_directive = ".ORGA";
string libRR_bank_directive = ".BANK";
string libRR_slot_directive = "SLOT";

string write_section_header(string offset_str, string bank_number, string section_name) {
    // first get the offset as int before appending "$"
    int32_t offset = hex_to_int(offset_str);
    string contents = "\n";
    offset_str = "$"+ offset_str;
    contents += libRR_bank_directive+" " + bank_number + " " + libRR_slot_directive+" "+get_slot_for_address(offset)+"\n";
    contents += libRR_org_directive+" "+offset_str;
    return contents+"\n";
}

void libRR_export_function_data() {

    string main_asm_contents = "";
    main_asm_contents+= write_console_asm_header();
    main_asm_contents+=  include_directive+" \"./common/constants.asm\"\n";

    // Loop through each bank
    for (auto& bank : libRR_called_functions.items()) {

      // Now loop over functions inside bank
      for (auto& func : bank.value().items()) {
        string bank_str = bank.key();
        string func_offset_str = func.key();
        int32_t func_offset = hex_to_int(func_offset_str);
        string export_path = get_function_export_path(func_offset_str, func.value(), bank_str);

        // if (func_offset >= 0xff80) {
        //   main_asm_contents += "; Ignore HRAM for now\n\n";
        //   continue;
        // }
        
        main_asm_contents+=include_directive+" \"."+export_path+"\"\n";

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

    main_asm_contents+=include_directive+" \"jumps.asm\"\n";
    main_asm_contents+=include_directive+" \"unwritten_relative_jumps.asm\"\n";

    // Generate main.asm file, which includes the other files
    codeDataLogger::writeStringToFile(libRR_export_directory+"main.asm", main_asm_contents);
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