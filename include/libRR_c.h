#ifndef LIBRETRO_REVERSING_C_H__
#define LIBRETRO_REVERSING_C_H__

#ifdef __cplusplus
extern "C" {
#endif



void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total);
retro_input_state_t libRR_handle_input(retro_input_state_t input_cb);

// Main
bool libRR_run_frame();


#ifdef __cplusplus
}
#endif

#endif