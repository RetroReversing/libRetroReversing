#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;
#include "CDL.hpp"


json fileConfig;
json reConfig;

std::map<uint32_t,uint32_t> rsp_reads;
std::map<uint32_t,uint32_t> rdram_reads;
std::map<uint32_t,cdl_tlb> tlbs;
std::map<uint32_t,cdl_dma> dmas = std::map<uint32_t,cdl_dma>();
std::map<uint32_t,char> jumps;
std::map<uint32_t,string> audio_address;
std::map<uint32_t, cdl_dram_cart_map> audio_samples;
std::map<uint32_t, cdl_dram_cart_map> cart_rom_dma_writes;
std::map<uint32_t, cdl_dram_cart_map> dma_sp_writes;
std::map<uint32_t, cdl_labels> labels; // Deprecated: Should now use functions
// cdl_labels functions is not playthrough specific
std::map<uint32_t, cdl_labels> functions;

std::map<uint32_t, cdl_jump_return> jump_returns;
std::map<string, string> function_signatures;
std::map<uint32_t,uint8_t> cached_jumps;
std::map<uint32_t,string> memory_to_log;

void readJsonToObject(string filename, json& json_object) {
    // printf("readJsonToObject %s \n", filename.c_str());
    std::ifstream i(filename);
    if (!i.good()) {
        json_object = json::parse("{}");
        return;
    } 
    json temp_json = {};
    try {
        i >> temp_json;
    } catch(...) {
		std::cerr << "WARNING: json error with file:" << filename << std::endl;
        json_object = json::parse("{}");
        return;
	}
    if (temp_json == NULL) {
        return;
    }
    string dump = temp_json.dump();
    if (temp_json != NULL && dump != "null") {
        // printf("Loading %s as: %s \n", dump.c_str(), filename.c_str());
        json_object = temp_json;
    }
}

void saveJsonToFile(string filename, json& json_object) {
    cout << "Saving: " << filename << std::endl;
    string dump = json_object.dump(2);
    if (dump == "null" || dump == "{}") {
        return;
    }
    std::ofstream o(filename);
    if (o.good()) {
        o << dump;
    } else {
        cout << "ERROR: Failed to write file: " << filename << std::endl;
    }
    cout << "Save Successful" << std::endl; 
}

void save_dram_rw_to_json() {
    // Note if you save it here you might want to also update readJsonFromFile()
    fileConfig["rsp_reads"] = rsp_reads;
    fileConfig["rdram_reads"] = rdram_reads;
    fileConfig["tlbs"] = tlbs;
    fileConfig["dmas"] = dmas;
    fileConfig["jumps"] = jumps;
    //fileConfig["jump_data"] = jump_data;
    fileConfig["audio_samples"] = audio_samples;
    fileConfig["cart_rom_dma_writes"] = cart_rom_dma_writes;
    fileConfig["dma_sp_writes"] = dma_sp_writes;
    fileConfig["labels"] = labels;
    fileConfig["function_signatures"] = function_signatures;
    fileConfig["cached_jumps"] = cached_jumps;
    fileConfig["memory_to_log"] = memory_to_log;
    fileConfig["audio_address"] = audio_address;
    // fileConfig["addresses"] = addresses;
}