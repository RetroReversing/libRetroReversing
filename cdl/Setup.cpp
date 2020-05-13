#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"

// Variables
int libRR_should_Load_EPROM = 0;
player_settings libRR_settings = { .paused = false};


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

// Settings
string libRR_parse_message_from_web(string message) {
  printf("New Web Message %s \n", message.c_str());
  
  auto message_json = json::parse(message);
  string category = message_json["category"].get<std::string>();
  if (category == "player_settings") {
    printf("Player settings!!\n");
    player_settings p2 =  message_json["state"].get<player_settings>();
    std::cout << p2.paused << std::endl;
    libRR_settings = p2;
  }
  // std::cout << message_json.dump(4) << std::endl;
  return "{ 'success':true }";
}

