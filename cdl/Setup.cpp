#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"
#include <filesystem>

// Variables
libRR_frame_buffer libRR_current_frame_buffer = {};
bool libRR_should_playback_input = true;
bool libRR_should_log_input = false;
int libRR_last_logged_frame = 0;
string current_playthrough_directory = "";
json game_json = {};
json playthroughs_json = {};
json libRR_current_playthrough = {};
string libRR_project_directory = "";
string libRR_current_playthrough_name = "Initial Playthrough";
int libRR_should_Load_EPROM = 0;
int libRR_message_duration_in_frames = 180;
player_settings libRR_settings = {.paused = true, .playbackLogged = false, .recordInput = false};

std::map<string, libRR_emulator_state> playthroughs = {};
libRR_emulator_state current_state = {};
retro_environment_t environ_cb = {};

std::vector<libRR_save_state> libRR_save_states = {};

// 
// External Libretro variables
// 
extern char retro_save_directory[4096];
extern char retro_base_directory[4096];
extern  char retro_cd_base_directory[4096];
extern  char retro_cd_path[4096];
extern char retro_cd_base_name[4096];

void save_playthough_metadata();

void init_playthrough(string name) {
  // 
  // Create Playthough directory if it doesn't already exist
  // 
  current_playthrough_directory = libRR_project_directory+ "/playthroughs/"+name+"/";
  std::__fs::filesystem::create_directories( current_playthrough_directory );

  readJsonToObject(current_playthrough_directory+"/playthrough.json", libRR_current_playthrough);
  save_playthough_metadata();
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin");
  libRR_last_logged_frame = libRR_current_playthrough["last_frame"];
  libRR_should_playback_input = true;
  printf("Loaded last logged frame: %d\n",libRR_last_logged_frame);
}

void libRR_define_console_memory_region(string name, unsigned long long start, unsigned long long end, long long mirror_address)
{
  cout << name << "\n";
}

void libRR_get_list_of_memory_regions()
{
  // can we save the memory map to json and send to client?
  std::vector<retro_memory_descriptor> memory_descriptors;
  for (int i = 0; i < libRR_retromap.num_descriptors; i++)
  {
    // printf("MMAP: %d %s \n", i, libRR_retromap.descriptors[i].addrspace);
    if (libRR_retromap.descriptors[i].ptr != NULL)
    {
      memory_descriptors.push_back(libRR_retromap.descriptors[i]);
    } else {
      // printf("Memory for %s is NULL \n", libRR_retromap.descriptors[i].addrspace);
    }
  }

  current_state.memory_descriptors = memory_descriptors;
  game_json["current_state"] = current_state;
}

void libRR_setup_directories() {
  libRR_project_directory = retro_base_directory; 
  libRR_project_directory+= "/RE_projects/";
  libRR_project_directory+=current_state.libretro_system_info.library_name;
  libRR_project_directory+="/"+current_state.game_name+"/";
  // std::__fs::filesystem::create_directories( path+"/RE_projects/"+current_state.libretro_system_info.library_name+"/"+current_state.game_name+"/");
  std::__fs::filesystem::create_directories( libRR_project_directory);
  std::__fs::filesystem::create_directories( libRR_project_directory+ "/playthroughs/");
  cout << "Created project directory: " << libRR_project_directory << std::endl;
}

void read_json_config() {
  readJsonToObject(libRR_project_directory+"/game.json", game_json);
  cout << game_json.dump(4) << std::endl;
  readJsonToObject(libRR_project_directory+"/playthroughs.json", playthroughs_json);
  cout << playthroughs_json.dump(4) << std::endl;
}

void libRR_handle_load_game(const struct retro_game_info *info, retro_environment_t _environ_cb)
{
  environ_cb = _environ_cb;
  printf("Loading a new ROM \n");
  libRR_setup_console_details(environ_cb);
  
  current_state.game_name = retro_cd_base_name;
  current_state.libretro_game_info = *info;
  current_state.libRR_save_states = libRR_save_states;

  retro_get_system_av_info(&current_state.libretro_video_info);
  retro_get_system_info(&current_state.libretro_system_info);
  printf("\n\nFPS: %f \n", current_state.libretro_video_info.timing.fps);
  libRR_get_list_of_memory_regions();

  current_state.paths.retro_save_directory = retro_save_directory;
  current_state.paths.retro_base_directory = retro_base_directory;
  current_state.paths.retro_cd_base_directory = retro_cd_base_directory;
  current_state.paths.retro_cd_path = retro_cd_path;
  current_state.paths.retro_cd_base_name = retro_cd_base_name;
  // 
  // Setup reversing files
  // 
  read_json_config();
  libRR_setup_directories();
  init_playthrough("Initial Playthrough"); // todo get name from front end
  game_json["current_state"] = current_state;
  setup_web_server();
}

void libRR_handle_emulator_close()
{
  stop_web_server();
}

bool libRR_write_binary_data_to_file(uint8_t * data, size_t len, string file_name) {
        std::ofstream file(file_name, std::ios_base::binary);
        file.write(reinterpret_cast <char*> (data),len);
        file.close();
        return !file.fail();
}

bool libRR_read_binary_data_from_file(uint8_t * data, size_t len, string file_name) {
        std::ifstream file(file_name, std::ios_base::binary);
        file.read(reinterpret_cast <char*> (data),len);
        file.close();
        return !file.fail();
}

void libRR_set_framebuffer(const void *fb, unsigned int length, unsigned int width, unsigned int height, unsigned int pitch) {
  // printf("set framebuffer: %d \n", fb[0]);
  if (libRR_current_frame_buffer.fb == NULL) {
    libRR_current_frame_buffer.fb = malloc(length);
  }
  memcpy((void*)libRR_current_frame_buffer.fb, fb, length);
  libRR_current_frame_buffer.length = length;
  libRR_current_frame_buffer.width = width;
  libRR_current_frame_buffer.height = height;
  libRR_current_frame_buffer.pitch = pitch;
}

void save_playthough_metadata() {
  if (libRR_current_playthrough.count("name") < 1) {
    libRR_current_playthrough["name"] = "Initial Playthrough";
    libRR_current_playthrough["states"] =  json::parse("[]");
    libRR_current_playthrough["current_state"] =  json::parse("{}");
    libRR_current_playthrough["last_frame"] =  0;
  }
  saveJsonToFile(current_playthrough_directory+"/playthrough.json", libRR_current_playthrough);
}

void libRR_reset(unsigned int reset_frame) {
  RRCurrentFrame = reset_frame;
  libRR_should_playback_input = true;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", reset_frame);
}

string libRR_load_save_state(int frame) {
  size_t length_of_save_buffer = retro_serialize_size();
  uint8_t *data = (uint8_t *)malloc(length_of_save_buffer);
  string filename = "save_"+to_string(frame)+".sav";
  libRR_read_binary_data_from_file(data, length_of_save_buffer, current_playthrough_directory+filename);
  retro_unserialize(data, length_of_save_buffer);
  RRCurrentFrame = frame;
  libRR_should_playback_input = true;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", frame);

  return libRR_current_playthrough.dump(4);
} 

string libRR_create_save_state(string name, int frame) {

  string filename = "save_"+to_string(frame)+".sav";

  // Create Save state
  size_t length_of_save_buffer = retro_serialize_size();
  uint8_t *data = (uint8_t *)malloc(length_of_save_buffer);
  retro_serialize(data, length_of_save_buffer);
  cout << "Length of save buffer: " << length_of_save_buffer << " Name:" << name << std::endl;
  libRR_write_binary_data_to_file(data, length_of_save_buffer, current_playthrough_directory+filename);
  free(data);

  // Save screenshot
  libRR_create_png(current_playthrough_directory+filename+".png", libRR_current_frame_buffer);

  // Update History
  libRR_save_state state = {};
  state.name = name;
  state.frame = RRCurrentFrame;
  libRR_current_playthrough["states"].push_back(state);
  libRR_current_playthrough["current_state"] = state;
  

  // current_state.libRR_save_states.push_back(state);
  // current_state.current_state = state;

  if (RRCurrentFrame > libRR_current_playthrough["last_frame"]) {
      // current_state.last_frame = RRCurrentFrame;
      libRR_current_playthrough["last_frame"] = RRCurrentFrame;
  }
  save_playthough_metadata();

  // TODO: save input log here
  if (!libRR_should_playback_input) {
    libRR_save_button_state_to_file(current_playthrough_directory+"button_log.bin");
  }

  // json json_save_states = current_state.libRR_save_states;
  return libRR_current_playthrough.dump(4);
}

string get_memory_for_web(string memory_name, int offset, int length)
{
  printf("get_memory_for_web: %s \n", memory_name.c_str());

  for (auto &i : current_state.memory_descriptors)
  {
    if (i.addrspace == memory_name)
    {
      int end = i.start + i.len;
      if (i.start + offset >= end)
      {
        // Starting at the end is no good
        return "[]";
      }
      if ((i.start + offset + length) >= end)
      {
        length = end - (i.start + offset);
      }
      return printBytesToDecimalJSArray((uint8_t *)(i.ptr) + offset, length);
    }
  }

  for (auto &i : libRR_cd_tracks)
  {
    if (i.name == memory_name) {
      int end = i.length;
      if (offset >= end)
      {
        return "[]"; // Starting at the end is no good so just return
      }
      if ((offset + length) >= end)
      {
        length = end - (offset);
      }
      return printBytesToDecimalJSArray((uint8_t *)(i.data) + offset, length);
    }
  }

  return "[]";
}

void libRR_display_message(const char *format, ...)
{
  va_list ap;
  struct retro_message msg;
  const char *strc = NULL;
  char *str = (char *)malloc(4096 * sizeof(char));

  va_start(ap, format);

  vsnprintf(str, 4096, format, ap);
  va_end(ap);
  strc = str;

  msg.frames = libRR_message_duration_in_frames;
  msg.msg = strc;

  environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
  free(str);
}

// Settings
string libRR_parse_message_from_web(string message)
{
  // printf("New Web Message %s \n", message.c_str());

  auto message_json = json::parse(message);
  string category = message_json["category"].get<std::string>();
  
  libRR_get_list_of_memory_regions();

  if (category == "player_settings")
  {
    printf("OLD Player settings!\n");
    // player_settings p2 = message_json["state"].get<player_settings>();
    // std::cout << p2.paused << std::endl;
    // libRR_settings = p2;
    return game_json.dump(4);
  }
  else if (category == "request_memory")
  {
    printf("Request for memory %s\n", message_json["state"]["name"].dump(4).c_str());
    return get_memory_for_web(message_json["state"]["memory"]["name"], message_json["state"]["offset"], message_json["state"]["length"]);
  }
  else if (category == "stop") {
    // retro_unload_game();
    retro_deinit();
    exit(0);
  }
  else if (category == "play") {
    printf("Play!\n");
    player_settings p2 = message_json["state"].get<player_settings>();
    libRR_settings = p2;
    
    if (libRR_current_playthrough["last_frame"] != 0) {
      // std::cout << p2.dump(4) << std::endl;
      std::cout << "Would load:" << libRR_current_playthrough["current_state"]["frame"].dump(4) << std::endl;
      // libRR_load_save_state(libRR_current_playthrough["current_state"]["frame"]);
    }

    return game_json.dump(4);
  }
  else if (category == "pause") {
    printf("Pause!\n");
    player_settings p2 = message_json["state"].get<player_settings>();
    std::cout << p2.paused << std::endl;
    libRR_settings = p2;
    return game_json.dump(4);
  }
  else if (category == "restart") {
    libRR_reset(0);
    retro_reset();
  }
  else if (category == "save_state") {
    return libRR_create_save_state(message_json["state"]["name"], RRCurrentFrame);
  }
  else if (category == "load_state") {
    libRR_reset(0);
    return libRR_load_save_state(message_json["state"]["frame"]);
  }
  else
  {
    printf("Unknown category %s with state: %s\n", category.c_str(), message_json["state"].dump(4).c_str());
  }

  libRR_display_message("Category: %s", category.c_str());

  // Update game_json based on emulator settings
  game_json["current_state"] = current_state;
  game_json["playthrough"] = libRR_current_playthrough;
  game_json["cd_tracks"] = libRR_cd_tracks;
  
  return game_json.dump(4);

}
