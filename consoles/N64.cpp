#include "../cdl/CDL.hpp"


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