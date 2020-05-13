#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"

// Variables
int libRRshouldLogInput=0;


void libRR_define_console_memory_region(string name, unsigned long long start, unsigned long long end, long long mirror_address) {
  cout << name << "\n";
}

void libRR_handle_load_game() {
  printf("Loading a new ROM \n");
  libRR_setup_console_details();
  setup_web_server();
  if (libRRshouldLogInput == 0) {
  //   read_button_state_from_file();
  }
}

void libRR_handle_emulator_close() {
  if (libRRshouldLogInput == 1) {
    // save_button_state_to_file();
  }
  stop_web_server();
}