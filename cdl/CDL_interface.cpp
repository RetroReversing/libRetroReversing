#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../cdl/CDL_FileWriting.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

using namespace std;

#include "CDL.hpp"

extern json fileConfig;
extern json reConfig;
extern json playthough_function_usage;
extern json libRR_console_constants;

// json libultra_signatures;
json linker_map_file;
#define USE_CDL 1;

extern bool cdl_log_memory;

extern std::map<uint32_t,string> memory_to_log;
extern std::map<uint32_t,char> jumps;
extern std::map<uint32_t,string> audio_address;
extern std::map<uint32_t,uint8_t> cached_jumps;
// std::map<uint32_t, uint8_t*> jump_data;
extern std::map<uint32_t,uint32_t> rsp_reads;
extern std::map<uint32_t,uint32_t> rdram_reads;
// std::map<uint32_t,bool> offsetHasAssembly;

extern "C" {
    // TODO: move the following includes, they are for N64
// #include "../main/rom.h"
// #include "../device/r4300/tlb.h"
// #include "../device/r4300/r4300_core.h"
// #include "../device/memory/memory.h"
// TODO: need to log input and then call input.keyDown(keymod, keysym);

// 
// # Variables
// 
string rom_name = "UNKNOWN_ROM"; // ROM_PARAMS.headername
int corrupt_start =  0xb2b77c;
int corrupt_end = 0xb2b77c;
int difference = corrupt_end-corrupt_start;


bool libRR_finished_boot_rom = false;
// string last_reversed_address = "";
// bool should_reverse_jumps = false;
// bool should_change_jumps = false;
int frame_last_reversed = 0;
int time_last_reversed = 0;
string libRR_game_name = "";
string libRR_rom_name = "";

// string next_dma_type = "";
// uint32_t previous_function = 0;
 std::vector<uint32_t> function_stack = std::vector<uint32_t>();
std::vector<uint32_t> previous_ra; // previous return address

std::map<uint32_t, std::map<string, string> > addresses;

void saveJsonToFile() {
    string filename = "./configs/";
    filename += rom_name;
    filename += ".json";
    std::ofstream o(filename);
    o << fileConfig.dump(1) << std::endl;
}

void show_interface() {
    int answer;
    std::cout << "1) Reset ROM 2) Change corrupt number ";
    std::cin >> std::hex >> answer;
    // if (answer == 1) {
    //     std::cout << "Resetting ROM";
    //     resetCart();
    // }
    // else {
    //     std::cout << "Unknown command";
    // }
    printf("Answer: %d \n", answer);
}

void corrupt_if_in_range(uint8_t* mem, uint32_t proper_cart_address) {
    // if (proper_cart_address >= corrupt_start && proper_cart_address <= corrupt_end) { //l_CurrentFrame == 0x478 && length == 0x04) { //} proper_cart_address == 0xb4015c) {
    //     printf("save_state_before\n");
    //     main_state_save(0, "before_corruption");
    //     printBytes(mem, proper_cart_address);
    //     printf("MODIFIED IT!! %#08x\n\n\n", mem[proper_cart_address+1]);
    //     corruptBytes(mem, proper_cart_address, 10);
    //     printBytes(mem, proper_cart_address);
    // }
}

void corruptBytes(uint8_t* mem, uint32_t cartAddr, int times) {
    #ifndef USE_CDL
        return;
    #endif
    if (times>difference) {
        times=difference/4;
    }
    // srand(time(NULL));  //doesn't work on windows
    printf("Corrupt Start: %d End: %d Difference: %d \n", corrupt_start, corrupt_end, difference);
    int randomNewValue = rand() % 0xFF;
    for (int i=0; i<=times; i++) {
        int randomOffset = rand() % difference;
        int currentOffset = randomOffset;
        printf("Offset: %d OldValue: %#08x NewValue: %#08x \n", currentOffset, mem[cartAddr+currentOffset], randomNewValue);
        mem[cartAddr+currentOffset] = randomNewValue;
    }
}

void cdl_log_opcode(uint32_t program_counter, uint8_t* op_address) {
    // only called in pure_interp mode
        // jump_data[program_counter] = op_address;
        // if (!labels[function_stack.back()].generatedSignature) {
        //     printf("Not generated sig yet: %#08x \n", *op_address);
        // }
    
}

uint32_t map_assembly_offset_to_rom_offset(uint32_t assembly_offset, uint32_t tlb_mapped_addr) {
    // or if its in KSEG0/1
    if (assembly_offset >= 0x80000000) {
        uint32_t mapped_offset = assembly_offset & UINT32_C(0x1ffffffc);
        // std::cout << "todo:" << std::hex << assembly_offset << "\n";
        return map_assembly_offset_to_rom_offset(mapped_offset, assembly_offset);
    }

    for(auto it = tlbs.begin(); it != tlbs.end(); ++it) {
        auto t = it->second;
        if (assembly_offset>=t.start && assembly_offset <=t.end) {
            uint32_t mapped_offset = t.rom_offset + (assembly_offset-t.start);
            return map_assembly_offset_to_rom_offset(mapped_offset, assembly_offset);
        }
    }
    for(auto it = dmas.begin(); it != dmas.end(); ++it) {
        auto& t = it->second;
        if (assembly_offset>=t.dram_start && assembly_offset <=t.dram_end) {
            uint32_t mapped_offset = t.rom_start + (assembly_offset-t.dram_start);
            t.is_assembly = true;
            t.tbl_mapped_addr = tlb_mapped_addr;
            // DMA is likely the actual value in rom
            return mapped_offset;
        }
    }
    // std::cout << "Not in dmas:" << std::hex << assembly_offset << "\n";
    // std::cout << "Unmapped: " << std::hex << assembly_offset << "\n";
    return assembly_offset;
}

int reverse_jump(int take_jump, uint32_t jump_target) {
    // this function doesn't work on windows
    // time_t now = time(0);
    // string key = n2hexstr(jump_target);          
    // printf("Reversing jump %#08x %d \n", jump_target, jumps[jump_target]);
    // take_jump = !take_jump;
    // time_last_reversed = now;
    // frame_last_reversed=l_CurrentFrame;
    // last_reversed_address = key;
    // fileConfig["reversed_jumps"][key] = jumps[jump_target];
    // write_rom_mapping();
    return take_jump;
}

void cdl_log_jump_cached(int take_jump, uint32_t jump_target, uint8_t* jump_target_memory) {
    if (cached_jumps.find(jump_target) != cached_jumps.end() ) 
        return;
    cached_jumps[jump_target] = 1;
    cout << "Cached:" << std::hex << jump_target << "\n";
}

int number_of_functions = 0;
bool libRR_full_function_log = false;

bool libRR_full_trace_log = false; // used in Genesis core in functions such as libRR_log_trace_str

int last_return_address = 0;
uint32_t libRR_call_depth = 0; // Tracks how big our stack trace is in terms of number of function calls

// We store the stackpointers in backtrace_stackpointers everytime a function gets called
uint16_t libRR_backtrace_stackpointers[0x200]; // 0x200 should be tons of function calls
uint32_t libRR_backtrace_size = 0; // Used with backtrace_stackpointers - Tracks how big our stack trace is in terms of number of function calls

extern uint32_t libRR_pc_lookahead;

string current_trace_log = "";
const int trace_messages_until_flush = 40;
int current_trace_count = 0;
bool first_trace_write = true;
void libRR_log_trace_str(string message) {
    
    if (!libRR_full_trace_log) {
        return;
    }
    current_trace_log += message + "\n";
    current_trace_count++;
    if (current_trace_count >= trace_messages_until_flush) {
        libRR_log_trace_flush();
        current_trace_count = 0;
        current_trace_log="";
    }
}
void libRR_log_trace(const char* message) {
    libRR_log_trace_str(message);
}

void libRR_log_trace_flush() {
    if (!libRR_full_trace_log) {
        return;
    }
    string output_file_path = libRR_export_directory + "trace_log.txt";
    if (first_trace_write) {
        codeDataLogger::writeStringToFile(output_file_path, current_trace_log);
        first_trace_write = false;
    } else {
        codeDataLogger::appendStringToFile(output_file_path, current_trace_log);
    }
}

// libRR_log_return_statement
// stack_pointer is used to make sure our function stack doesn't exceed the actual stack pointer
// used in cores: Gameboy
void libRR_log_return_statement(uint32_t current_pc, uint32_t return_target, uint32_t stack_pointer) {
    if (libRR_full_trace_log) {
        libRR_log_trace_str("Return:"+n2hexstr(current_pc)+"->"+n2hexstr(return_target));
    }
    // printf("libRR_log_return_statement pc:%d return:%d stack:%d\n", current_pc, return_target, 65534-stack_pointer);

    // check the integrety of the call stack
    if (libRR_call_depth < 0) {
        printf("Function seems to have returned without changing the stack, PC: %d \n", current_pc);
    }

    auto function_returning_from = function_stack.back();
    auto presumed_return_address = previous_ra.back();
    if (return_target != presumed_return_address) {
        // printf("ERROR: Presumed return: %d actual return: %d current_pc: %d\n", presumed_return_address, return_target, current_pc);
        // sometimes code manually pushes the ret value to the stack and returns
        // if so we don't want to log as a function return
        // but in the future we might want to consider making the previous jump a function call
        return;
    } else {
        libRR_call_depth--;
        // Remove from stacks
        function_stack.pop_back();
        previous_ra.pop_back();
    }

    if (!libRR_full_function_log) {
        return;
    }
    
    current_pc -= libRR_pc_lookahead; 
    string current_function = n2hexstr(function_returning_from);
    string current_pc_str = n2hexstr(current_pc);
    // printf("Returning from function: %s current_pc:%s \n", current_function.c_str(), current_pc_str.c_str());
    // string function_key = current_function;
    playthough_function_usage[current_function]["returns"][current_pc_str] = return_target;

    // Add max return to functions
    if (functions.find(function_returning_from) != functions.end() ) {
        uint32_t relative_return_pc = current_pc - function_returning_from;
        if (relative_return_pc > functions[function_returning_from].return_offset_from_start) {
            functions[function_returning_from].return_offset_from_start = relative_return_pc;
        }
    }

    // TODO: Calculate Function Signature so we can check for its name
    int length = current_pc - function_returning_from;
    string length_str = n2hexstr(length);
    playthough_function_usage[current_function]["lengths"][length_str] = length;
    if (length > 0 && length < 200) {
        if (playthough_function_usage[current_function]["signatures"].contains(length_str)) {

        } else {
            printf("Function Signature: About to get length: %d \n", length);
            playthough_function_usage[current_function]["signatures"][n2hexstr(length)] = libRR_get_data_for_function(function_returning_from, length+1, true, true);
        }
    }    

    // string bytes_with_branch_delay = printBytesToStr(jump_data[previous_function_backup], byte_len+4)+"_"+n2hexstr(length+4);
    // string word_pattern = printWordsToStr(jump_data[previous_function_backup], byte_len+4)+" L"+n2hexstr(length+4,4);
    // TODO: need to get the moment where the bytes for the function are located 
    // printf("Logged inst: %s \n", name.c_str());
}



// libRR_log_full_function_call is expensive as it does extensive logging
void libRR_log_full_function_call(uint32_t current_pc, uint32_t jump_target) {
    // Instead of using function name, we just use the location
    string function_name = /*game_name + "_func_" +*/ n2hexstr(jump_target);
    // printf("libRR_log_full_function_call Full function logging on %s \n", print_function_stack_trace().c_str());


    // This is playthough specific
    if (!playthough_function_usage.contains(function_name)) {
        // printf("Adding new function %s \n", function_name.c_str());
        playthough_function_usage[function_name] = json::parse("{}");
        playthough_function_usage[function_name]["first_frame_access"] = RRCurrentFrame;
        playthough_function_usage[function_name]["number_of_frames"]=0;
        playthough_function_usage[function_name]["last_frame_access"] = 0;
        playthough_function_usage[function_name]["number_of_calls_per_frame"] = 1;
    }
    else if (RRCurrentFrame < playthough_function_usage[function_name]["last_frame_access"]) {
        // we have already ran this frame before, probably replaying, no need to add more logging
        return;
    }

    if (RRCurrentFrame > playthough_function_usage[function_name]["last_frame_access"]) {
        playthough_function_usage[function_name]["last_frame_access"] = RRCurrentFrame;
        playthough_function_usage[function_name]["number_of_frames"]= (int)playthough_function_usage[function_name]["number_of_frames"]+1;
        playthough_function_usage[function_name]["number_of_calls_per_frame"]=0;
    } 
    else if (RRCurrentFrame == playthough_function_usage[function_name]["last_frame_access"]) {
        // we are in the same frame so its called more than once per frame
        playthough_function_usage[function_name]["number_of_calls_per_frame"]=(int)playthough_function_usage[function_name]["number_of_calls_per_frame"]+1;
    }

    // TODO: log read/writes to memory
    // TODO: calculate return and paramerters
    // TODO: find out how long the function is
}

const char* libRR_log_long_jump(uint32_t current_pc, uint32_t jump_target, const char* type) {
    // cout << "Long Jump from:" << n2hexstr(current_pc) << " to:" << n2hexstr(jump_target) << "\n";
    if (libRR_full_trace_log) {
        libRR_log_trace_str("Long Jump:"+n2hexstr(current_pc)+"->"+n2hexstr(jump_target)+" type:"+type);
    }

    string target_bank_number = "0000";
    string pc_bank_number = "0000";
    target_bank_number = n2hexstr(get_current_bank_number_for_address(jump_target), 4);

    // now we need the bank number of the function we are calling
    pc_bank_number = n2hexstr(get_current_bank_number_for_address(current_pc), 4);
    libRR_long_jumps[target_bank_number][n2hexstr(jump_target)][pc_bank_number+"::"+n2hexstr(current_pc)]=type;
    return libRR_log_jump_label(jump_target, current_pc);
}

void libRR_log_interrupt_call(uint32_t current_pc, uint32_t jump_target) {
    string pc_bank_number = "0000";
    pc_bank_number = n2hexstr(get_current_bank_number_for_address(current_pc), 4);

    // printf("Interrupt call at: %s::%s target:%s \n", pc_bank_number.c_str(), n2hexstr(current_pc).c_str(), n2hexstr(jump_target).c_str());
    libRR_long_jumps["0000"][n2hexstr(jump_target)][pc_bank_number+"::"+n2hexstr(current_pc)]=true;
}

// Restarts are very similar to calls but can only jump to specific targets and only take up 1 byte
void libRR_log_rst(uint32_t current_pc, uint32_t jump_target) {
    // for now just log it as a standard function call
    libRR_log_function_call(current_pc, jump_target, 0x00);
}

string function_name = ""; // last function name called
const char* libRR_log_function_call(uint32_t current_pc, uint32_t jump_target, uint32_t stack_pointer) {
    // TODO: find out why uncommeting the following causes a segfault
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return "nolog";
    }
    string bank_number = "0000";
    uint32_t calculated_jump_target = jump_target;
    if (libRR_bank_switching_available) {
        int bank = get_current_bank_number_for_address(jump_target);
        bank_number = n2hexstr(bank, 4);

        // TODO: the following might be gameboy specific
        if (jump_target >= libRR_bank_size) {
            // printf("TODO: remove this in gameboy!\n");
            calculated_jump_target = jump_target + ((bank-1) * libRR_bank_size);
        }
        // END TODO
    }

    string jump_target_str = n2hexstr(jump_target);
    function_name = "_"+bank_number+"_func_"+jump_target_str;
    libRR_called_functions[bank_number][n2hexstr(jump_target)] = function_name;
    libRR_log_trace_str("Function call: 0x"+jump_target_str);
    
    // Start Stacktrace handling
    libRR_call_depth++;
    // End Stacktrace handling
    
    last_return_address = current_pc;
    function_stack.push_back(jump_target);
    previous_ra.push_back(current_pc);
    if (libRR_full_function_log) {
        libRR_log_full_function_call(current_pc, jump_target);
    }
    if (functions.find(jump_target) != functions.end() ) {
        // We have already logged this function, so ignore for now
        return function_name.c_str();
    }
    // We have never logged this function so lets create it
    auto t = cdl_labels();
    t.func_offset = n2hexstr(calculated_jump_target);
    // if (functions.find(previous_function_backup) != functions.end()) {
    //     t.caller_offset = functions[previous_function_backup].func_name+" (ra:"+n2hexstr(ra)+")";
    // } else {
    //     t.caller_offset = n2hexstr(previous_function_backup);
    // }
    t.func_name = function_name; // /*libRR_game_name+*/"_"+bank_number+"_func_"+jump_target_str;
    //t.func_stack = function_stack.size();
    //t.export_path = "";
    //t.bank_number = bank_number;
    //t.bank_offset = jump_target;
    // t.stack_trace = print_function_stack_trace();
    t.doNotLog = false;
    t.many_memory_reads = false;
    t.many_memory_writes = false;
    // t.additional["callers"][print_function_stack_trace()] = RRCurrentFrame;
    printf("Logged new function: %s target:%d number_of_functions:%d \n", t.func_name.c_str(), jump_target, number_of_functions);
    functions[jump_target] = t;
    number_of_functions++;
    return function_name.c_str();
}

// loop through and erse multiple functions
void erase_multiple_func_signatures() {
    // function_signatures
    // Multiple functions
}

bool is_auto_generated_function_name(string func_name) {
    if (func_name.find("_func_") != std::string::npos) {
                // this is a non renamed function as it was auto generated
                return true;
    }
    return false;
}


} // end extern C

// C++

uint32_t libRR_offset_to_look_for = 0x8149;
bool libRR_enable_look = false;

json libRR_disassembly = {};
json libRR_memory_reads = {};
json libRR_consecutive_rom_reads = {};
json libRR_called_functions = {};
json libRR_long_jumps = {};
int32_t previous_consecutive_rom_read = 0; // previous read address to check if this read is part of the chain
int16_t previous_consecutive_rom_bank = 0; // previous read address to check if this read is part of the chain
int32_t current_consecutive_rom_start = 0; // start address of the current chain

bool replace(std::string& str, const std::string from, const std::string to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

extern "C" void libRR_log_dma(int32_t offset) {
    if (offset> 0x7fff) {
        return;
    }
    cout << "DMA: " << n2hexstr(offset) << "\n";

}

static string label_name = "";
extern "C" const char* libRR_log_jump_label_with_name(uint32_t offset, uint32_t current_pc, const char* label_name) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return "";
    }
    
    string offset_str = n2hexstr(offset);
    int bank = get_current_bank_number_for_address(offset);
    string current_bank_str = n2hexstr(bank, 4);
    
    if (offset >libRR_slot_2_max_addr) {
        // if its greater than the max bank value then its probably in ram
        return "";
    }

    // debugging code start
    if (libRR_enable_look && (offset == libRR_offset_to_look_for || current_pc == libRR_offset_to_look_for)) {
        printf("Found long jump label with name offset: %d\n", libRR_offset_to_look_for);
    }
    // debugging code end

    // string label_name = "LAB_" + current_bank_str + "_" + n2hexstr(offset);
    if (!libRR_disassembly[current_bank_str][offset_str].contains("label_name")) {
        libRR_disassembly[current_bank_str][offset_str]["label_name"] = label_name;
    }
    libRR_disassembly[current_bank_str][offset_str]["meta"]["label_callers"][current_bank_str + "_" + n2hexstr(current_pc)] = true;
    return label_name;
}


extern "C" const char* libRR_log_jump_label(uint32_t offset, uint32_t current_pc) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return "_not_logging";
    }
    
    string offset_str = n2hexstr(offset);
    int bank = get_current_bank_number_for_address(offset);
    string current_bank_str = n2hexstr(bank, 4);
    
    if (offset >libRR_slot_2_max_addr) {
        // if its greater than the max bank value then its probably in ram
        return "max_bank_value";
    }

    // debugging code start
    if (libRR_enable_look && (offset == libRR_offset_to_look_for || current_pc == libRR_offset_to_look_for)) {
        printf("Found long jump label offset: %d\n", libRR_offset_to_look_for);
    }
    // debugging code end

    label_name = "LAB_" + current_bank_str + "_" + n2hexstr(offset);
    // return libRR_log_jump_label_with_name(offset, current_pc, label_name.c_str());

    if (!libRR_disassembly[current_bank_str][offset_str].contains("label_name")) {
        libRR_disassembly[current_bank_str][offset_str]["label_name"] = label_name;
    }
    libRR_disassembly[current_bank_str][offset_str]["meta"]["label_callers"][current_bank_str + "_" + n2hexstr(current_pc)] = true;
    return label_name.c_str();
}

extern "C" void libRR_log_memory_read(int8_t bank, int32_t offset, const char* type, uint8_t byte_size, char* bytes) {
    libRR_log_rom_read(bank, offset, type, byte_size, bytes);
}

extern "C" void libRR_log_rom_read(int16_t bank, int32_t offset, const char* type, uint8_t byte_size, char* bytes) {
    string bank_str = n2hexstr(bank, 4);
    string previous_bank_str = n2hexstr(previous_consecutive_rom_bank, 4);
    string offset_str = n2hexstr(offset);
    string current_consecutive_rom_start_str = n2hexstr(current_consecutive_rom_start);
    if (libRR_full_trace_log) {
        libRR_log_trace_str("Rom Read bank:"+bank_str+":"+n2hexstr(offset)+" = "+n2hexstr(bytes[0], 2));
    }
    // Check to see if the last read address is the same or 1 away
    // Check for the same is because sometimes data is checked by reading the first byte
    if (previous_consecutive_rom_bank == bank && previous_consecutive_rom_read == offset) {
        // do nothing if its the same byte read twice
        previous_consecutive_rom_bank = bank;
        libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["length"] = 1;
        return;
    }
    if (previous_consecutive_rom_bank == bank && previous_consecutive_rom_read == (offset-1)) {
        if (libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str].is_null()) {
            // check to see if the current read is null and if so create it
            libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["length"] = 1+ byte_size;
        } else {
            libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["length"] = ((uint32_t) libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["length"]) +byte_size;
        }
        for (int i=0; i<byte_size; i++) {
            libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["value"][n2hexstr(offset+i)] = n2hexstr(bytes[i]);
        }
    } 
    else {
        // cout << "previous consecutive length from:" << (int)previous_consecutive_rom_bank << "::" << n2hexstr(current_consecutive_rom_start) << " -> " << n2hexstr(previous_consecutive_rom_read) <<  " len:" << libRR_consecutive_rom_reads[previous_bank_str][current_consecutive_rom_start_str]["length"] << "\n";
        current_consecutive_rom_start = offset;
        current_consecutive_rom_start_str = n2hexstr(current_consecutive_rom_start);
        // initialise new consecutive run
        libRR_consecutive_rom_reads[bank_str][current_consecutive_rom_start_str]["length"] = 1;
        for (int i=0; i<byte_size; i++) {
            libRR_consecutive_rom_reads[bank_str][current_consecutive_rom_start_str]["value"][n2hexstr(offset+i)] = n2hexstr(bytes[i]);
        }
    }
    previous_consecutive_rom_read = offset+(byte_size-1); // add byte_size to take into account 2 byte reads
    previous_consecutive_rom_bank = bank;

    string value_str = "";
    if (byte_size == 2) {
        value_str = n2hexstr(two_bytes_to_16bit_value(bytes[1], bytes[0]));
    } else {
         value_str = n2hexstr(bytes[0]);
    }
    // printf("Access data: %d::%s type: %s size: %d value: %s\n", bank, n2hexstr(offset).c_str(), type, byte_size, value_str.c_str());
}

extern "C" void libRR_log_instruction_2int(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand, uint32_t operand2) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    std::string name(c_name);
    replace(name, "%int%", libRR_constant_replace(operand));
    replace(name, "%int2%", libRR_constant_replace(operand2));
    
    libRR_log_instruction(current_pc, name, instruction_bytes, number_of_bytes);
}
// Takes a single int argument and replaces it in the string
extern "C" void libRR_log_instruction_1int(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand) {
    return libRR_log_instruction_2int(current_pc, c_name, instruction_bytes, number_of_bytes, operand, 0);
}

extern "C" void libRR_log_instruction_1string(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, const char* c_register_name) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    std::string name(c_name);
    std::string register_name(c_register_name);
    replace(name, "%str%",register_name);
    libRR_log_instruction_1int(current_pc, name.c_str(), instruction_bytes, number_of_bytes, 0x00);
}
extern "C" void libRR_log_instruction_1int_registername(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand, const char* c_register_name) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    std::string name(c_name);
    std::string register_name(c_register_name);
    replace(name, "%r%",register_name);
    libRR_log_instruction_1int(current_pc, name.c_str(), instruction_bytes, number_of_bytes, operand);
}

extern "C" void libRR_log_instruction_z80_s_d(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, const char* source, const char* destination) {
     if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    std::string name(c_name);
    replace(name, "%s%", source);
    replace(name, "%d%", destination);
    
    libRR_log_instruction(current_pc, name, instruction_bytes, number_of_bytes);
}


// 
// Z80 End
// 

// current_pc - current program counter
// instruction bytes as integer used for hex
// arguments  - number of arguments - currently not really used for anything
// m - used for register number, replaces Rm with R1/R2 etc
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int number_of_bytes, unsigned m, unsigned n, unsigned imm, unsigned d, unsigned ea) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    replace(name, "%EA", "0x"+n2hexstr(ea));
    libRR_log_instruction(current_pc, name, instruction_bytes, number_of_bytes, m, n, imm, d);
}
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int number_of_bytes, unsigned m, unsigned n, unsigned imm, unsigned d) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    replace(name, "#imm", "#"+to_string(imm));
    replace(name, "disp", ""+to_string(d));
    if (name.find("SysRegs") != std::string::npos) {
        replace(name, "SysRegs[#0]", "MACH");
        replace(name, "SysRegs[#1]", "MACL");
        replace(name, "SysRegs[#2]", "PR");
    }
    libRR_log_instruction(current_pc, name, instruction_bytes, number_of_bytes, m, n);
}


void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int number_of_bytes, unsigned m, unsigned n) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    replace(name, "Rm", "R"+to_string(m));
    replace(name, "Rn", "R"+to_string(n));
    libRR_log_instruction(current_pc, name, instruction_bytes, number_of_bytes);
}

extern "C" void libRR_log_instruction(uint32_t current_pc, const char* name, uint32_t instruction_bytes, int number_of_bytes)
{
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }
    // printf("libRR_log_instruction pc:%d name: %s bytes: %d\n", current_pc, name, instruction_bytes);
    std::string str(name);
    libRR_log_instruction(current_pc, str, instruction_bytes, number_of_bytes);
}

// C version of the c++ template
extern "C" const char* n2hexstr_c(int number, size_t hex_len) {
    return n2hexstr(number, hex_len).c_str();
}

string libRR_constant_replace(uint32_t da8) {
    string addr_str = n2hexstr(da8);
    if (libRR_console_constants["addresses"].contains(addr_str)) {
        return libRR_console_constants["addresses"][addr_str];
    }
    return "$"+n2hexstr(da8);
}

int32_t previous_pc = 0; // used for debugging
bool has_read_first_ever_instruction = false;
void libRR_log_instruction(uint32_t current_pc, string name, uint32_t instruction_bytes, int number_of_bytes) {
    if (!libRR_full_function_log || !libRR_finished_boot_rom) {
        return;
    }

    if (!has_read_first_ever_instruction) {
        // special handling for the entry point, we wanr to force a label here to it gets written to output
        libRR_log_jump_label_with_name(current_pc, current_pc, "entry");
        has_read_first_ever_instruction = true;
        libRR_isDelaySlot = false;
    }

    int bank = get_current_bank_number_for_address(current_pc);
    string current_bank_str = n2hexstr(bank, 4);

    // trace log each instruction
    if (libRR_full_trace_log) {
        libRR_log_trace_str(name + "; pc:"+current_bank_str+":"+n2hexstr(current_pc));
    }

    // Code used for debugging why an address was reached
    if (libRR_enable_look && current_pc == libRR_offset_to_look_for) {
        printf("Reached %s: previous addr: %s name:%s bank:%d \n ", n2hexstr(libRR_offset_to_look_for).c_str(), n2hexstr(previous_pc).c_str(), name.c_str(), bank);
    }
    // end debugging code
    
    if (strcmp(libRR_console,"Saturn")==0) {
        printf("isSaturn\n");
        // For saturn we remove 2 from the program counter, but this will vary per console
        current_pc -= 4; // was -2
    }


    // string current_function = n2hexstr(function_stack.back());
    string current_pc_str = n2hexstr(current_pc);
    // printf("libRR_log_instruction %s \n", current_function.c_str());
    if (strcmp(libRR_console,"Saturn")==0) {
        if (libRR_isDelaySlot) {
            current_pc_str = n2hexstr(libRR_delay_slot_pc - 2); //subtract 2 as pc is ahead
            // printf("Delay Slot %s \n", current_pc_str.c_str());
            libRR_isDelaySlot = false;
        }
    }

    // TODO: Hex bytes should change based on number_of_bytes
    string hexBytes = n2hexstr((uint32_t)instruction_bytes, number_of_bytes*2);
    

    // if we are below the max addr of bank 0 (e.g 0x4000 for GB) then we are always in bank 0
    // if (current_pc <libRR_slot_0_max_addr) {
    //     current_bank_str="0000";
    // }

    // libRR_disassembly[current_bank_str][current_pc_str][name]["frame"]=RRCurrentFrame;
    libRR_disassembly[current_bank_str][current_pc_str][name]["bytes"]=hexBytes;
    libRR_disassembly[current_bank_str][current_pc_str][name]["bytes_length"]=number_of_bytes;
    previous_pc = current_pc;
}