#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"

// Variables
int libRR_should_Load_EPROM = 0;
player_settings libRR_settings = { .paused = true, .playbackLogged = false, .recordInput = false};

libRR_emulator_state current_state = {};
// retro_system_av_info libretro_video_info = {};


void libRR_define_console_memory_region(string name, unsigned long long start, unsigned long long end, long long mirror_address) {
  cout << name << "\n";
}

void libRR_get_list_of_memory_regions() {
  // can we save the memory map to json and send to client?
  std::vector<retro_memory_descriptor> memory_descriptors;
  for (int i=0; i<libRR_retromap.num_descriptors; i++) {
    printf("MMAP: %d %s \n", i, libRR_retromap.descriptors[i].addrspace);
    if (libRR_retromap.descriptors[i].ptr != NULL) {
      memory_descriptors.push_back(libRR_retromap.descriptors[i]);
    }
  }
  current_state.memory_descriptors = memory_descriptors;
}

void libRR_handle_load_game(const struct retro_game_info *info, retro_environment_t environ_cb) {
  printf("Loading a new ROM \n");
  libRR_setup_console_details(environ_cb);
  setup_web_server();
  if (libRR_settings.playbackLogged) {
    libRR_read_button_state_from_file();
  }

  current_state.libretro_game_info = *info;
  // current_state.libretro_map = libRR_retromap;
  retro_get_system_av_info(&current_state.libretro_video_info);
  retro_get_system_info(&current_state.libretro_system_info);
  printf("\n\nFPS: %f \n",current_state.libretro_video_info.timing.fps);
  libRR_get_list_of_memory_regions();
}

void libRR_handle_emulator_close() {
  if (libRR_settings.recordInput) {
    libRR_save_button_state_to_file();
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
  printf("\n\nAspect Ratio: %f \n",current_state.libretro_video_info.geometry.aspect_ratio);

  json j = current_state;
  return j.dump(4);

  // return "{ 'success':true }";
}

