#ifndef LIBRETRO_REVERSING_C_H__
#define LIBRETRO_REVERSING_C_H__
#include "./libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* libRR_console;

extern bool libRR_full_function_log;
extern bool libRR_finished_boot_rom;

// ASM instruction logging
void libRR_log_instruction(uint32_t current_pc, const char* name, uint32_t instruction_bytes, int number_of_bytes);
void libRR_log_instruction_z80_register(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint8_t opcode, uint16_t operand, const char* register_name);
void libRR_log_instruction_z80(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, uint8_t opcode, uint16_t operand);
void libRR_log_instruction_z80_s_d(uint32_t current_pc, const char* c_name, uint32_t instruction_bytes, int number_of_bytes, const char* source, const char* destination);
const char* libRR_log_jump_label(int32_t offset);

void libRR_set_retro_memmap(struct retro_memory_descriptor* descs, int num_descriptors);
extern struct retro_memory_descriptor libRR_mmap[12];
extern int libRR_mmap_descriptors;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
struct retro_input_state_t libRR_handle_input(struct retro_input_state_t input_cb);
extern struct retro_memory_map libRR_retromap;

// Main
bool libRR_run_frame();


#ifdef __cplusplus
}
#endif

#endif