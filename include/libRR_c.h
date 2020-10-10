#ifndef LIBRETRO_REVERSING_C_H__
#define LIBRETRO_REVERSING_C_H__

#ifdef __cplusplus
extern "C" {
#endif

void libRR_set_retro_memmap(struct retro_memory_descriptor* descs, int num_descriptors);
extern struct retro_memory_descriptor libRR_mmap[12];
extern int libRR_mmap_descriptors;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
retro_input_state_t libRR_handle_input(retro_input_state_t input_cb);
extern struct retro_memory_map libRR_retromap;

// Main
bool libRR_run_frame();


#ifdef __cplusplus
}
#endif

#endif