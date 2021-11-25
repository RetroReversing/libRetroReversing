#include "../cdl/CDL.hpp"
#include "../source_exporter/CommonSourceExport.h"
#include "../cdl/nlohmann/json.hpp"
using json = nlohmann::json;
json linker_map_file;
extern json fileConfig;
extern json reConfig;
extern std::map<uint32_t,string> memory_to_log;
extern std::map<uint32_t,char> jumps;
extern std::map<uint32_t,string> audio_address;
extern std::map<uint32_t,uint8_t> cached_jumps;
std::map<uint32_t, uint8_t*> jump_data;
extern std::map<uint32_t,uint32_t> rsp_reads;
extern std::map<uint32_t,uint32_t> rdram_reads;
std::map<uint32_t,bool> offsetHasAssembly;
std::vector<uint32_t> function_stack = std::vector<uint32_t>();
std::vector<uint32_t> previous_ra; // previous return address
string ucode_crc = "";


 // 
// # Toggles
// 
bool support_n64_prints = false;
bool cdl_log_memory = false;
bool tag_functions = false;
bool log_notes = false;
bool log_function_calls = false;
bool log_ostasks = false;
bool log_rsp = false;
bool should_reverse_jumps = false;

// Functions declared in this file
void readLibUltraSignatures();

// start from mupen
typedef struct
 {
    char goodname[256];
    char MD5[33];
    unsigned char savetype;
    unsigned char status;  /* Rom status on a scale from 0-5. */
    unsigned char players; /* Local players 0-4, 2/3/4 way Netplay indicated by 5/6/7. */
    unsigned char rumble;  /* 0 - No, 1 - Yes boolean for rumble support. */
 } m64p_rom_settings;
extern m64p_rom_settings   ROM_SETTINGS;

typedef enum
{
    SYSTEM_NTSC = 0,
    SYSTEM_PAL,
    SYSTEM_MPAL
} m64p_system_type;

typedef struct _rom_params
{
   char *cheats;
   m64p_system_type systemtype;
   char headername[21];  /* ROM Name as in the header, removing trailing whitespace */
   unsigned char countperop;
   int disableextramem;
   unsigned int sidmaduration;
} rom_params;
extern rom_params  ROM_PARAMS;

typedef struct
{
   uint8_t  init_PI_BSB_DOM1_LAT_REG;  /* 0x00 */
   uint8_t  init_PI_BSB_DOM1_PGS_REG;  /* 0x01 */
   uint8_t  init_PI_BSB_DOM1_PWD_REG;  /* 0x02 */
   uint8_t  init_PI_BSB_DOM1_PGS_REG2; /* 0x03 */
   uint32_t ClockRate;                 /* 0x04 */
   uint32_t PC;                        /* 0x08 */
   uint32_t Release;                   /* 0x0C */
   uint32_t CRC1;                      /* 0x10 */
   uint32_t CRC2;                      /* 0x14 */
   uint32_t Unknown[2];                /* 0x18 */
   uint8_t  Name[20];                  /* 0x20 */
   uint32_t unknown;                   /* 0x34 */
   uint32_t Manufacturer_ID;           /* 0x38 */
   uint16_t Cartridge_ID;              /* 0x3C - Game serial number  */
   uint16_t Country_code;              /* 0x3E */
} m64p_rom_header;
extern m64p_rom_header   ROM_HEADER;
// end from Mupen

string mapping_names[] = {
    "M64P_MEM_NOTHING",
    "M64P_MEM_NOTHING",
    "M64P_MEM_RDRAM",
    "M64P_MEM_RDRAMREG",
    "M64P_MEM_RSPMEM",
    "M64P_MEM_RSPREG",
    "M64P_MEM_RSP",
    "M64P_MEM_DP",
    "M64P_MEM_DPS",
    "M64P_MEM_VI",
    "M64P_MEM_AI",
    "M64P_MEM_PI",
    "M64P_MEM_RI",
    "M64P_MEM_SI",
    "M64P_MEM_FLASHRAMSTAT",
    "M64P_MEM_ROM",
    "M64P_MEM_PIF",
    "M64P_MEM_MI"
};

const char *register_names[] = {
    "$r0",
    "$at",
    "v0", "v1",
    "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9",
    "k0", "k1",
    "$gp",
    "$sp",
    "sB",
    "$ra"
};
#define REGISTER_A0 4
#define REGISTER_A1 5
#define REGISTER_A2 6
#define REGISTER_A3 7
#define REGISTER_GP 28
#define REGISTER_SP 29
#define REGISTER_SB  30
#define REGISTER_RA  31
uint32_t *fast_mem_access(struct r4300_core* r4300, uint32_t address);

json libultra_signatures;
string last_reversed_address = "";

void setTogglesBasedOnConfig() {
    cdl_log_memory = reConfig["shouldLogMemory"];
    tag_functions = reConfig["shouldTagFunctions"];
    log_notes = reConfig["shouldLogNotes"];
    log_function_calls = reConfig["shouldLogFunctionCalls"];
    support_n64_prints = reConfig["shouldSupportN64Prints"];
    log_ostasks = reConfig["shouldLogOsTasks"];
    log_rsp = reConfig["shouldLogRsp"];
}

void cdl_log_opcode_error() {
    printf("Very bad opcode, caused crash! \n");
    fileConfig["bad_jumps"][last_reversed_address] = "crash";
    main_state_load(NULL);
}

bool createdCartBackup = false;
void backupCart() {
   // libRR_game_name = alphabetic_only_name((char*)rom_name.c_str(), 21);
    std::cout << "TODO: backup";
    createdCartBackup = true;
}
void resetCart() {
    std::cout << "TODO: reset";
}

void readJsonFromFile() {
    readLibUltraSignatures();
    readJsonToObject("symbols.json", linker_map_file);
    readJsonToObject("./reconfig.json", reConfig);
    setTogglesBasedOnConfig();
    string filename = "./configs/";
    filename+=rom_name;
    filename += ".json";
    // read a JSON file
    if (!reConfig["startFreshEveryTime"]) {
        cout << "Reading previous game config file \n";
        readJsonToObject(filename, fileConfig);
    }
    if (fileConfig.find("jumps") == fileConfig.end()) {
                fileConfig["jumps"] = R"([])"_json;
    }
    if (fileConfig.find("tlbs") == fileConfig.end()) {
                fileConfig["tlbs"] = R"([])"_json;
    }
    if (fileConfig.find("dmas") == fileConfig.end()) {
                fileConfig["dmas"] = R"([])"_json;
    }
    if (fileConfig.find("rsp_reads") == fileConfig.end()) {
                fileConfig["rsp_reads"] = R"([])"_json;
    }
    if (fileConfig.find("rdram_reads") == fileConfig.end())
                fileConfig["rdram_reads"] = R"([])"_json;
    if (fileConfig.find("reversed_jumps") == fileConfig.end())
                fileConfig["reversed_jumps"] = R"({})"_json;
    if (fileConfig.find("labels") == fileConfig.end())
                fileConfig["labels"] = R"([])"_json;
    if (fileConfig.find("jump_returns") == fileConfig.end())
                fileConfig["jump_returns"] = R"([])"_json;
    if (fileConfig.find("memory_to_log") == fileConfig.end())
                fileConfig["memory_to_log"] = R"([])"_json;

    memory_to_log = fileConfig["memory_to_log"].get< std::map<uint32_t,string> >();
    memory_to_log[0x0E5320] = "rsp.boot";
    jumps = fileConfig["jumps"].get< std::map<uint32_t,char> >();
    tlbs = fileConfig["tlbs"].get< std::map<uint32_t,cdl_tlb> >();
    dmas = fileConfig["dmas"].get< std::map<uint32_t,cdl_dma> >();
    rsp_reads = fileConfig["rsp_reads"].get< std::map<uint32_t,uint32_t> >();
    rdram_reads = fileConfig["rdram_reads"].get< std::map<uint32_t,uint32_t> >();
    labels = fileConfig["labels"].get< std::map<uint32_t,cdl_labels> >();
    jump_returns = fileConfig["jump_returns"].get< std::map<uint32_t,cdl_jump_return> >();
}

void cdl_log_pif_ram(uint32_t address, uint32_t* value) {
    #ifndef USE_CDL
        return;
    #endif
    printf("Game was reset? \n");
    if (!createdCartBackup) {
        backupCart();
        readJsonFromFile();
        function_stack.push_back(0);
    }
    if (should_reverse_jumps) {
        // should_reverse_jumps = false;
        fileConfig["bad_jumps"][last_reversed_address] = "reset";
        main_state_load(NULL);
        write_rom_mapping();
    }
}

// these are all the data regions
// for assembly regions check out the tlb
string create_n64_split_regions(cdl_dma d) { //uint8_t* header_bytes, uint32_t proper_cart_address, uint32_t length, uint32_t dram_addr, uint32_t frame,  bool is_assembly, uint32_t tbl_mapped_addr) {
    // uint8_t* header_bytes = (uint8_t*)&d.header;
    uint32_t header_bytes = __builtin_bswap32(d.header);
    std::stringstream sstream, header;
    string region_type = "bin";
    if (d.is_assembly) {
        region_type="asm";
    }
    string proper_cart_address_str = n2hexstr(d.rom_start);
    string ascii_header = d.ascii_header;
    header << " header: " <<  ascii_header << " 0x" << std::hex << header_bytes; // (header_bytes[3]+0) << (header_bytes[2]+0) << (header_bytes[1]+0) << (header_bytes[0]+0);
    sstream << "  - [0x" << std::hex << d.rom_start << ", 0x"<< (d.rom_start+d.length);
    sstream << ", \"" << region_type << "\",   ";
    if (d.known_name.length()>1) {
        sstream << "\"" << d.known_name;
    }
    else if (strcmp(d.guess_type.c_str(), "audio") == 0) {
        sstream << "\"" << d.guess_type << "_" << d.rom_start << "_len_"<< d.length;
    } else {
        sstream << "\"_" << ascii_header << "_" << "_" << d.rom_start << "_len_"<< d.length;
    }

    if (d.is_assembly) {
        sstream << "\", 0x" << n2hexstr(d.dram_start) << "] # frame:0x" << n2hexstr(d.frame);
    } else {
        sstream << "\"] # (DRAM:0x" << n2hexstr(d.dram_start) << ") (frame:0x" << n2hexstr(d.frame) << ") ";
    }

    if (strcmp(d.guess_type.c_str(), "audio") != 0) {
        sstream << header.str() << " trace:" << d.func_addr;
    }
    //sstream << " Func:" << d.func_addr;
    
    if (d.tbl_mapped_addr>0) {
        sstream << " Tbl mapped:"<<d.tbl_mapped_addr;
    }

    std::string mapping = sstream.str();
    return mapping;
}

void resetReversing() {
    // time_last_reversed = time(0); // doesn;t work on windows
    last_reversed_address="";
}

void readLibUltraSignatures() {
    std::ifstream i("libultra.json");
    if (i.good()) {
        i >> libultra_signatures;
    } 
    if (libultra_signatures.find("function_signatures") == libultra_signatures.end()) {
            libultra_signatures["function_signatures"] = R"([])"_json;
    }
}
void saveLibUltraSignatures() {
    std::ofstream o("libultra.json");
    o << libultra_signatures.dump(1) << std::endl;
}

void save_cdl_files() {
    resetReversing();
    find_asm_sections();
    find_audio_sections();
    find_audio_functions();
    save_dram_rw_to_json();
    saveJsonToFile();
    saveLibUltraSignatures();
}

void write_rom_mapping() {
    save_cdl_files();
    printf("ROM_PARAMS.headername: %s \n", rom_name.c_str());
    string filename = "./configs/";
    filename+=rom_name;
    filename += ".config.yaml";
    ofstream file(filename, std::ios_base::binary);
    file << "# ROM splitter configuration file\n";
    file << "name: \"";
    file << ROM_SETTINGS.goodname;
    file << "\"\n";
    file << "# Graphics uCodeCRC: \"" << ucode_crc << "\"\n";
    file << "# checksums from ROM header offsets 0x10 and 0x14\n";
    file << "# used for auto configuration detection\n";
    file << "checksum1: 0x";
    file << std::hex << ROM_HEADER.CRC1;
    file <<"\nchecksum2: 0x";
    file << std::hex << ROM_HEADER.CRC2;
    file <<"\n# base filename used for outputs - (please, no spaces)\n";
    file <<"basename: \"";
    file << alphabetic_only_name(ROM_PARAMS.headername, 21);
    file << "\"\n";
    file <<"ranges:\n";
    file <<"  # start,  end,      type,     label\n";
    file <<"  - [0x000000, 0x000040, \"header\", \"header\"]\n";
    file <<"  - [0x000040, 0x000B70, \"asm\",    \"boot\"]\n";
    file <<"  - [0x000B70, 0x001000, \"bin\",    \"bootcode_font\"]\n";    

    //
    // Write out 
    //
    for (auto& it : dmas) {
        auto t = it.second;
        if (it.first ==0 || t.dram_start == 0) continue;
        file << create_n64_split_regions(t) << "\n";
    }

    file <<"# Labels for functions or data memory addresses\n";
    file <<"# All label addresses are RAM addresses\n";
    file <<"# Order does not matter\n";
    file <<"labels:\n";
    uint32_t entryPoint = ROM_HEADER.PC; // (int8_t*)(void*)&ROM_HEADER.PC;
    file << "   - [0x" << std::hex << __builtin_bswap32(entryPoint)+0 <<", \"EntryPoint\"]\n";
    for (auto& it : labels) {
        auto t = it.second;
        if (strcmp(t.func_offset.c_str(), "") == 0) continue;
        file << "   - [0x" << t.func_offset <<", \"" <<  t.func_name << "\"]\n";
    }

}

void console_log_jump_return(int take_jump, uint32_t jump_target, uint32_t pc, uint32_t ra, int64_t* registers, struct r4300_core* r4300) {
//   if (support_n64_prints) {
//     if (strcmp(labels[previous_function_backup].func_name.c_str(),"osSyncPrintf") ==0) {
//         uint32_t* memory = fast_mem_access(r4300, registers[REGISTER_A2]);
//         string swapped = string_endian_swap((const char*)memory);
//         labels[function_stack.back()].printfs[swapped] = "";
//         printf("\n%s > %s",labels[function_stack.back()].func_name.c_str(), swapped.c_str());
//     }
//   }
}

// TLB entry
struct tlb_entry
{
   short mask;
   unsigned int vpn2;
   char g;
   unsigned char asid;
   unsigned int pfn_even;
   char c_even;
   char d_even;
   char v_even;
   unsigned int pfn_odd;
   char c_odd;
   char d_odd;
   char v_odd;
   char r;
   //int check_parity_mask;

   unsigned int start_even;
   unsigned int end_even;
   unsigned int phys_even;
   unsigned int start_odd;
   unsigned int end_odd;
   unsigned int phys_odd;
};

void save_table_mapping(int entry, uint32_t phys, uint32_t start,uint32_t end, bool isOdd) {
    
    //printf("tlb_map:%d ODD Start:%#08x End:%#08x Phys:%#08x \n",entry, e->start_odd, e->end_odd, e->phys_odd);
        uint32_t length = end-start;

        auto t = cdl_tlb();
        t.start=start;
        t.end = end;
        t.rom_offset = phys;
        tlbs[phys]=t;

        string key = "";
        key+="[0x";
        key+=n2hexstr(phys);
        key+=", 0x";
        key+=n2hexstr(phys+length);
        key+="] Virtual: 0x";
        key+=n2hexstr(start);
        key+=" to 0x";
        key+=n2hexstr(end);
        if (isOdd)
        key+=" Odd";
        else
        key+=" Even";

        string value = "Entry:";
        value += to_string(entry);
        // value += " Frame:0x";
        value += n2hexstr(l_CurrentFrame);

        bool isInJson = fileConfig["tlb"].find(key) != fileConfig["tlb"].end();
        if (isInJson) {
            string original = fileConfig["tlb"][key];
            bool isSameValue = (strcmp(original.c_str(), value.c_str()) == 0);
            if (isSameValue) return;
            // printf("isSameValue:%d \noriginal:%s \nnew:%s\n", isSameValue, original.c_str(), value.c_str());
            return; // don't replace the original value as it is useful to match frame numbers to the mappings
        }
        fileConfig["tlb"][key] = value;
        printf("TLB %s\n", value.c_str());
}

// ASID: The asid argument specifies an address space identifier that makes the mappings valid only when a specific address space identifier register is loaded. (See osSetTLBASID.) A value of -1 for asid specifies a global mapping that is always valid.
// The CPU TLB consists of 32 entries, which provide mapping to 32 odd/even physical page pairs (64 total pages).
// Where is Page Size PM?
void log_tlb_entry(const struct tlb_entry* e, size_t entry) {
    // I think g is 1 when either odd or even is used
    // printf("tlb_map:%d MISC mask:%d vpn2:%#08x g:%d r:%d asid:%d \n",entry, e->mask, e->vpn2, e->g, e->r, e->asid);
    if (e->v_even) {
        save_table_mapping(entry, e->phys_even, e->start_even, e->end_even, false);
    }
    if (e->v_odd) {
        save_table_mapping(entry, e->phys_odd, e->start_odd, e->end_odd, true);
    }
}

// void cdl_log_memory_mappings(struct mem_mapping* mappings, uint32_t number_of_mappings) {
//     for (int i=0; i<=number_of_mappings; i++) {
//         std::stringstream s;
//         auto mapping = mappings[i];
//         s << "Mapping:" << mapping_names[mapping.type+0] << " " << std::hex << mapping.begin << "->" << mapping.end << "\n";
//         fileConfig["memMap"][i] = s.str();
//     }
// }

extern "C" {

 void libRR_direct_serialize(void *data, size_t size) {
    // TODO: implement this as a save function that is called internally just to save
    retro_serialize(data, size);
  }

  bool libRR_direct_unserialize(const void *data, size_t size) {
    // TODO: implement this properly
    retro_unserialize(data, size);
  }

  void libRR_export_all_files() {
    printf("N64: Export All files to Reversing Project, depends on which core we are using");
  }
}