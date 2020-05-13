#include <queue>
#include "../civetweb/include/civetweb.h"

// Common Variables
int l_CurrentFrame;

extern "C" {
  void console_log_jump_return(int take_jump, uint32_t jump_target, uint32_t pc, uint32_t ra, int64_t* registers, void* r4300) {
    printf("%d\n",1);
  }

  void main_state_load(const char *filename)
  {
    // TODO: actually load the state here
  }

  void main_state_save(int format, const char *filename)
  {
    // TODO: actually load the state here
  }

  void write_rom_mapping() {

  }

}