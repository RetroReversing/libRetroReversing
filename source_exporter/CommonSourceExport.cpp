#include <queue>
#include "../civetweb/include/civetweb.h"
#include "../include/libRR.h"
#include "../cdl/mustache.hpp"
#include "../cdl/CDL.hpp"
#include "../cdl/CDL_FileWriting.hpp"
using namespace kainjow::mustache;

string libRR_export_assembly_extention = ".s";

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
      std::__fs::filesystem::create_directories(path_to_create);
    }

    // Copy required files
    json j = files_json["files"];
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
      string current = *it;
      string contents = codeDataLogger::readFileToString(export_template_directory + current);

      mustache tmpl{contents};

      string output_file_path = libRR_export_directory + current;
      // create any folder that needs to be created
      std::__fs::filesystem::create_directories(codeDataLogger::dirnameOf(output_file_path));

      // Now save the file to the project directory
      codeDataLogger::writeStringToFile(output_file_path, 
        tmpl.render({"GAME_NAME", libRR_game_name}) );
      cout << "Written file to: " << output_file_path << "\n";

    }
  }