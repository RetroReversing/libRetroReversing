#ifndef LIBRETRO_REVERSING_C_H__
#define LIBRETRO_REVERSING_C_H__
#include "./libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* libRR_console;
extern int libRR_emulated_hardware; // used if the emulator core supports multiple consoles

extern bool libRR_full_function_log;
extern bool libRR_finished_boot_rom;
extern enum retro_pixel_format libRR_core_pixel_format;

// Bank Switching
extern uint16_t libRR_current_bank_slot_0;
extern uint16_t libRR_current_bank_slot_1;
extern uint16_t libRR_current_bank_slot_2;

int get_current_bank_number_for_address(uint32_t addr);

// Util
const char* n2hexstr_c(int number, size_t hex_len);
uint16_t two_bytes_to_16bit_value(uint8_t byte1, uint8_t byte2);
uint32_t three_bytes_to_24bit_value(uint8_t byte1, uint8_t byte2, uint8_t byte3);


// Trace logging
extern bool libRR_full_trace_log;
void libRR_log_trace(const char* message);
void libRR_log_trace_flush();

// DMA logging
void libRR_log_dma(int32_t offset);

// Jump logging
const char* libRR_log_long_jump(uint32_t current_pc, uint32_t jump_target, const char* type);

// Call logging
void libRR_log_rst(uint32_t current_pc, uint32_t jump_target);
const char* libRR_log_function_call(uint32_t current_pc, uint32_t target_pc, uint32_t stack_pointer);
void libRR_log_interrupt_call(uint32_t current_pc, uint32_t target_pc);
void libRR_log_return_statement(uint32_t current_pc, uint32_t return_target, uint32_t stack_pointer);


// ASM instruction logging
const char* libRR_asm_format();
void libRR_log_instruction(uint32_t current_pc, const char* name, uint32_t instruction_bytes, int number_of_bytes);
void libRR_log_instruction_1string(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, const char* name);
void libRR_log_instruction_1int_registername(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand, const char* register_name);
void libRR_log_instruction_1int(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand);
void libRR_log_instruction_2int(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint32_t operand, uint32_t operand2);
void libRR_log_instruction_z80_s_d(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, const char* source, const char* destination);
const char* libRR_log_jump_label(uint32_t offset, uint32_t current_pc);
void libRR_log_memory_read(int8_t bank, int32_t offset, const char* type, uint8_t byte_size, char* bytes);
void libRR_log_rom_read(int16_t bank, int32_t offset, const char* type, uint8_t byte_size, char* bytes);
void libRR_gameboy_log_memory_read(int32_t offset, const char* type, uint8_t byte_size, char* bytes);


void libRR_set_retro_memmap(struct retro_memory_descriptor* descs, int num_descriptors);
//extern struct retro_memory_descriptor libRR_mmap[12];
extern int libRR_mmap_descriptors;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
retro_input_state_t libRR_handle_input(retro_input_state_t input_cb);
extern struct retro_memory_map libRR_retromap;

// Main
bool libRR_run_frame();

// Save states
void libRR_create_save_state_c(const char* name, int frame, bool fast_save); // C version creating the save state and saving screenshot and metadata

#ifdef __cplusplus
}
#endif

#endif