#include "../cdl/CDL.hpp"
#include "../source_exporter/CommonSourceExport.h"

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

void write_rom_mapping() {
    save_cdl_files();
    printf("ROM_PARAMS.headername: %s \n", rom_name);
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
  if (support_n64_prints) {
    if (strcmp(labels[previous_function_backup].func_name.c_str(),"osSyncPrintf") ==0) {
        uint32_t* memory = fast_mem_access(r4300, registers[REGISTER_A2]);
        string swapped = string_endian_swap((const char*)memory);
        labels[function_stack.back()].printfs[swapped] = "";
        printf("\n%s > %s",labels[function_stack.back()].func_name.c_str(), swapped.c_str());
    }
}
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

void cdl_log_memory_mappings(mem_mapping* mappings, uint32_t number_of_mappings) {
    for (int i=0; i<=number_of_mappings; i++) {
        std::stringstream s;
        auto mapping = mappings[i];
        s << "Mapping:" << mapping_names[mapping.type+0] << " " << std::hex << mapping.begin << "->" << mapping.end << "\n";
        fileConfig["memMap"][i] = s.str();
    }
}

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