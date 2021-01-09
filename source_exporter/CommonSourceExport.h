#ifndef LIBRR_COMMON_SOURCE_EXPORT_H__
#define LIBRR_COMMON_SOURCE_EXPORT_H__

extern json allLabels;
void get_all_assembly_labels();
string write_callers(json callers);
void get_all_unwritten_labels();
string get_function_export_path(string offset, json func, string bank);
string get_function_name(string bank, string offset);
string write_asm_until_null(string bank_number, string offset_str, bool is_function);
bool should_stop_writing_asm(int start_offset, uint32_t i, string bank_number);
string write_bank_header_comment(string bank);
string write_section_header(string offset_str, string bank_number, string section_name);
void libRR_export_function_data();
void libRR_export_rom_data();
void libRR_export_jump_data();

extern string libRR_org_directive;
extern string libRR_bank_directive;
extern string libRR_slot_directive;
extern int libRR_total_banks;

// console specific
extern "C" {
  string get_slot_for_address(int32_t offset);
  string write_console_asm_header();
}

#endif