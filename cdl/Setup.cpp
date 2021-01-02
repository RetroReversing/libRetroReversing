#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"
#include <filesystem>

// Variables
char libRR_save_directory[4096];
libRR_frame_buffer libRR_current_frame_buffer = {};
unsigned int libRR_current_frame_buffer_length = 0;
bool libRR_should_playback_input = true;
bool libRR_should_log_input = false;
int libRR_last_logged_frame = 0;
string current_playthrough_directory = "";
json game_json = {};
json override_code_json = {};
json playthroughs_json = {};
json libRR_current_playthrough = {};
json playthough_function_usage = {};
json libRR_console_constants = {};
string libRR_project_directory = "";
string libRR_export_directory = "";
string libRR_current_playthrough_name = "Initial Playthrough";
int libRR_should_Load_EPROM = 0;
int libRR_message_duration_in_frames = 180;
player_settings libRR_settings = {.paused = true, .playbackLogged = false, .recordInput = false, .endAt = -1, .loopFrame = -1};

std::map<string, libRR_emulator_state> playthroughs = {};
libRR_emulator_state current_state = {};
retro_environment_t environ_cb = {};

std::vector<libRR_save_state> libRR_save_states = {};

// 
// External Libretro variables
// 
extern char retro_system_directory[4096];
extern char retro_base_directory[4096];
extern  char retro_cd_base_directory[4096];
extern  char retro_cd_path[4096];

void save_playthough_metadata();
void save_constant_metadata();

void init_playthrough(string name) {
  cout << "Init playthrough for " << name << std::endl;
  // 
  // Create Playthough directory if it doesn't already exist
  // 
  current_playthrough_directory = libRR_project_directory+ "/playthroughs/"+name+"/";
  std::__fs::filesystem::create_directories( current_playthrough_directory );

  cout << "About to read JSON files to memory" << std::endl;

  readJsonToObject(current_playthrough_directory+"/playthrough.json", libRR_current_playthrough);
  readJsonToObject(current_playthrough_directory+"/resources.json", game_json["cd_data"]["root_files"]);
  readJsonToObject(current_playthrough_directory+"/function_usage.json", playthough_function_usage);

  readJsonToObject(current_playthrough_directory+"/overrides.json", game_json["overrides"]);
  readJsonToObject(libRR_project_directory+"/notes.json", game_json["notes"]);
  readJsonToObject(libRR_project_directory+"/functions.json", game_json["functions"], "[]");
  readJsonToObject(libRR_project_directory+"/assembly.json", libRR_disassembly);
  readJsonToObject(libRR_project_directory+"/consecutive_rom_reads.json", libRR_consecutive_rom_reads);
  readJsonToObject(libRR_project_directory+"/called_functions.json", libRR_called_functions);
  readJsonToObject(libRR_project_directory+"/long_jumps.json", libRR_long_jumps);

  // Read static config that varies by console
  readJsonToObject("./constants/"+(string)libRR_console+".json", libRR_console_constants);
  cout << "About to set functions array" << std::endl;
  if (game_json.contains("functions") && game_json["functions"].dump() != "{}") {
    // cout << "FUNCTION JSON:" << game_json["functions"].dump() << std::endl;
    functions = game_json["functions"].get<std::map<uint32_t, cdl_labels>>();
  }

  cout << "About to save playthough metadata" << std::endl;
  save_playthough_metadata();
  cout << "About to save constant metadata" << std::endl;
  save_constant_metadata();
  cout << "About to read button state to memory" << std::endl;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", 0);
  if (!libRR_current_playthrough["last_frame"].is_null()) {
    libRR_last_logged_frame = libRR_current_playthrough["last_frame"];
  }
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
  // printf("libRR_get_list_of_memory_regions number:%d \n", libRR_retromap.num_descriptors);
  std::vector<retro_memory_descriptor> memory_descriptors;
  for (int i = 0; i < libRR_retromap.num_descriptors; i++)
  {
    // printf("MMAP: %d %s \n", i, libRR_retromap.descriptors[i].addrspace);
    if (libRR_retromap.descriptors[i].ptr != NULL)
    {
      memory_descriptors.push_back(libRR_retromap.descriptors[i]);
    } else {
      printf("Memory for %s is NULL \n", libRR_retromap.descriptors[i].addrspace);
    }
  }

  printf("Saving current_state.memory_descriptors\n");
  current_state.memory_descriptors = memory_descriptors;
  printf("Saving current_state\n");
  game_json["current_state"] = current_state;
}

void libRR_setup_retro_base_directory() {
  // Setup path
  const char *dir = NULL;

  if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir) {
        snprintf(libRR_save_directory, sizeof(libRR_save_directory), "%s", dir);
    }
  else {
      snprintf(libRR_save_directory, sizeof(libRR_save_directory), "%s", ".");
  }
  dir = NULL; 

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
   {
      snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
   }
   else {
     snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", libRR_save_directory);
   }
  // end setup path
}

void libRR_setup_directories() {

  libRR_setup_retro_base_directory();

  libRR_project_directory = retro_base_directory;
  libRR_project_directory += "/RE_projects/";
  libRR_project_directory += libRR_console; //current_state.libretro_system_info.library_name;
  libRR_project_directory += "/" + libRR_game_name + "/";
  libRR_export_directory += libRR_project_directory + "src/";
  std::__fs::filesystem::create_directories( libRR_project_directory);
  std::__fs::filesystem::create_directories( libRR_project_directory + "/playthroughs/");
  std::__fs::filesystem::create_directories( libRR_export_directory);
  cout << "Created project directory: " << libRR_project_directory << std::endl;
}

// 
// # Read all JSON config
// This isn't used yet
// 
void read_json_config() {
  readJsonToObject(libRR_project_directory+"/game.json", game_json);
  cout << game_json.dump(4) << std::endl;
  readJsonToObject(libRR_project_directory+"/playthroughs.json", playthroughs_json);
  cout << playthroughs_json.dump(4) << std::endl;
}

// TODO: move extract_basename to some sort of file/path utils
string extract_basename(const char *path)
{
  char buf[4096];
  size_t size = sizeof(buf);
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';

  return buf;
}

extern string libRR_game_name;
void libRR_handle_load_game(const struct retro_game_info *info, retro_environment_t _environ_cb)
{
  environ_cb = _environ_cb;
  printf("Loading a new ROM \n");
  libRR_setup_console_details(environ_cb);

  current_state.game_name = extract_basename(info->path);
  printf("Game path: %s name: %s\n", info->path, current_state.game_name.c_str());

  libRR_game_name = alphabetic_only_name((char*)current_state.game_name.c_str(), current_state.game_name.length());
  current_state.libretro_game_info = *info;
  current_state.libRR_save_states = libRR_save_states;

  retro_get_system_av_info(&current_state.libretro_video_info);
  retro_get_system_info(&current_state.libretro_system_info);
  printf("\n\nFPS: %f \n", current_state.libretro_video_info.timing.fps);
  libRR_get_list_of_memory_regions();

  current_state.paths.retro_save_directory = libRR_save_directory;
  current_state.paths.retro_base_directory = retro_base_directory;
  current_state.paths.retro_cd_base_directory = retro_cd_base_directory;
  current_state.paths.retro_cd_path = retro_cd_path;
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

bool libRR_delete_file(string file_name) {
  const int result = remove( file_name.c_str() );
  if( result == 0 ){
      printf( "successfully deleted: %s\n", file_name.c_str() );
      return true;
  } else {
      printf( "Error deleting: %s error: %s\n",file_name.c_str(), strerror( errno ) ); // No such file or directory
      return false;
  }

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

extern retro_video_refresh_t video_cb;
void libRR_video_cb(const void *fb, unsigned int width, unsigned int height, unsigned int pitch) {
  unsigned int length = pitch * height;
  video_cb(fb, width, height, pitch);
  libRR_set_framebuffer(fb, length, width, height, pitch);
}

// fb -0 the framebuffer object from libretro
// length of the full frame buffer
// width of the screen
// height of the screen
// pitch if applicable
void libRR_set_framebuffer(const void *fb, unsigned int length, unsigned int width, unsigned int height, unsigned int pitch) {
  if (libRR_current_frame_buffer.fb == NULL) {
    printf("set framebuffer length: %d width: %d \n", length, width);
    libRR_current_frame_buffer.fb = malloc(length);
    libRR_current_frame_buffer_length = length;
  } else if (libRR_current_frame_buffer_length < length) {
    printf("set framebuffer NEW Size length: %d width: %d \n", length, width);
    free(libRR_current_frame_buffer.fb);
    libRR_current_frame_buffer.fb = malloc(length);
    libRR_current_frame_buffer_length = length;
  }
  memcpy((void*)libRR_current_frame_buffer.fb, fb, length);
  libRR_current_frame_buffer.length = length;
  libRR_current_frame_buffer.width = width;
  libRR_current_frame_buffer.height = height;
  libRR_current_frame_buffer.pitch = pitch;
}

void save_constant_metadata() {
  printf("Save Constant Game Meta Data");
  // These files are Game specific rather than playthrough specific
  saveJsonToFile(libRR_project_directory+"/notes.json", game_json["notes"]);
  saveJsonToFile(libRR_project_directory+"/assembly.json", libRR_disassembly);
  saveJsonToFile(libRR_project_directory+"/consecutive_rom_reads.json", libRR_consecutive_rom_reads);
  saveJsonToFile(libRR_project_directory+"/called_functions.json", libRR_called_functions);
  saveJsonToFile(libRR_project_directory+"/long_jumps.json", libRR_long_jumps);

  cout << "About to save trace log" << std::endl;
  libRR_log_trace_flush();
  game_json["functions"] = functions;
  saveJsonToFile(libRR_project_directory+"/functions.json", game_json["functions"]);
}

void save_playthough_metadata() {
  printf("Save Playthough Meta Data");
  if (libRR_current_playthrough.count("name") < 1) {
    libRR_current_playthrough["name"] = "Initial Playthrough";
    libRR_current_playthrough["states"] =  json::parse("[]");
    libRR_current_playthrough["current_state"] =  json::parse("{}");
    libRR_current_playthrough["last_frame"] =  0;
  }
  saveJsonToFile(current_playthrough_directory+"/playthrough.json", libRR_current_playthrough);
  saveJsonToFile(current_playthrough_directory+"/resources.json", game_json["cd_data"]["root_files"]);
  saveJsonToFile(current_playthrough_directory+"/overrides.json", game_json["overrides"]);
  saveJsonToFile(current_playthrough_directory+"/function_usage.json", playthough_function_usage);
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

string libRR_delete_save_state(int frame) {
  string filename = current_playthrough_directory+"save_"+to_string(frame)+".sav";
  string png_filename = filename+".png";
  libRR_delete_file(filename);
  libRR_delete_file(png_filename);

  json j = libRR_current_playthrough["states"];
  int i=0;
  json next_latest_state;
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    json current = *it;
    if (current["frame"] == frame) {
      // Delete this frame
      current["frame"] = -1;
      libRR_current_playthrough["states"].erase(i);
    } else if (next_latest_state.is_null() || current["frame"]>next_latest_state["frame"]) {
      // if the one we are deleting is latest then we need to find the next latest
      next_latest_state = current;
    }
    i++;
  }

  int latest_state_number = libRR_current_playthrough["current_state"]["frame"];
  if (frame == latest_state_number && !next_latest_state.is_null()) {
      // User is deleting the last known state so we need special handling
      libRR_current_playthrough["current_state"] = next_latest_state;
      libRR_current_playthrough["last_frame"] = next_latest_state["frame"];
      printf("Since we are deleting the latest state, we will go back to: %d \n", (int)next_latest_state["frame"]);
      // next we want to remove some entries from the button log
      libRR_resave_button_state_to_file(current_playthrough_directory+"button_log.bin", (int)next_latest_state["frame"]);
  }

  save_playthough_metadata();
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
  string screenshot_name = current_playthrough_directory+filename+".png";
  libRR_create_png(screenshot_name, libRR_current_frame_buffer);

  // Update History
  libRR_save_state state = {};
  state.name = name;
  state.frame = RRCurrentFrame;
  libRR_current_playthrough["states"].push_back(state);
  libRR_current_playthrough["current_state"] = state;

  if (RRCurrentFrame > libRR_current_playthrough["last_frame"]) {
      libRR_current_playthrough["last_frame"] = RRCurrentFrame;
  }
  save_playthough_metadata();
  save_constant_metadata();

  if (!libRR_should_playback_input) {
    libRR_save_button_state_to_file(current_playthrough_directory+"button_log.bin");
  }

  // json json_save_states = current_state.libRR_save_states;
  return libRR_current_playthrough.dump(4);
}

string libRR_get_data_for_file(int offset, int length, bool swapEndian);


uint8_t* get_memory_pointer(string memory_name, int offset, int length) {
  if (memory_name == "file") {
    return NULL;
    // return libRR_get_data_for_file(offset, length);
  }
  for (auto &i : current_state.memory_descriptors)
  {
    if (i.addrspace == memory_name)
    {
      int end = i.start + i.len;
      if (i.start + offset >= end)
      {
        // Starting at the end is no good
        return NULL;
      }
      if ((i.start + offset + length) >= end)
      {
        length = end - (i.start + offset);
      }
      return (uint8_t *)(i.ptr) + offset;
    }
  }

  for (auto &i : libRR_cd_tracks)
  {
    if (i.name == memory_name) {
      int end = i.length;
      if (offset >= end)
      {
        return NULL; // Starting at the end is no good so just return
      }
      if ((offset + length) >= end)
      {
        length = end - (offset);
      }
      return (uint8_t *)(i.data) + offset;
    }
  }
}

string get_strings_for_web(string memory_name, int offset, int length) {
  uint8_t* memory = get_memory_pointer(memory_name, offset, length);
  string current_string = "";
  json found_strings;
  json valid_string_characters;
  int minimum_string_length = 4;

  return "Strings go here";
}

string libRR_get_data_for_function(int offset, int length, bool swapEndian, bool asHexString = false) {
  // printf("libRR_get_data_for_function offset: %d length: %d \n", offset, length);
  for (auto &i : current_state.memory_descriptors)
  {
    int end = i.start + i.len;
    if (offset >= i.start && offset < end) {
      int relative_offset = offset - i.start;
      if ((offset + length) >= end)
      {
        length = end - (offset);
      }
      
      if (asHexString) {
        return printBytesToStr((uint8_t *)(i.ptr) + relative_offset, length, swapEndian);
      }
      // printf("Found Name: %s start: %d end: %d \n", i.addrspace, i.start, end);
      return printBytesToDecimalJSArray((uint8_t *)(i.ptr) + relative_offset, length, swapEndian);
    }
  }
  printf("libRR_get_data_for_function Failed to find: %d \n", offset);
  return "Failed to find data";
}

string get_memory_for_web(string memory_name, int offset, int length, bool swapEndian)
{
  printf("get_memory_for_web: %s \n", memory_name.c_str());
  if (memory_name == "file") {
    return libRR_get_data_for_file(offset, length, swapEndian);
  } else if (memory_name == "function") {
    printf("Memory name function \n");
    return libRR_get_data_for_function(offset, length, swapEndian);
  }
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
      return printBytesToDecimalJSArray((uint8_t *)(i.ptr) + offset, length, swapEndian);
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
      return printBytesToDecimalJSArray((uint8_t *)(i.data) + offset, length, swapEndian);
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

void save_updates_to_function_json() {
  game_json["functions"] = functions;
  printf("Saving functions.json \n");
  saveJsonToFile(libRR_project_directory+"/functions.json", game_json["functions"]);
}

void edit_function(json state) {
  for (auto& it : functions) {
    if (it.second.func_offset == state["func_offset"]) {
      printf("Found function to update %s \n", state["func_offset"].dump().c_str());
      functions[it.first].func_name = state["func_name"];
      functions[it.first].export_path = state["export_path"];
      // functions[it.first].additional = message_json["state"]["additional"];
      break;
    }
  }
  save_updates_to_function_json();
}

void upload_linker_map(json linker_map) {
  // This function currently renames all the functions based on the sym file
  for (auto& it : functions) {
    string function_offset = it.second.func_offset;
    if (linker_map["libraryFunctions"].contains(function_offset)) {
      // linker_map[function_offset];
      printf("Found: %s \n", function_offset.c_str());
      functions[it.first].func_name = linker_map["libraryFunctions"][function_offset]["name"];
    }
    else {
      // printf("Can't find Function: %s \n", function_offset.c_str());
    }
    
  }
  printf("About to save updated to functions.json \n");
  save_updates_to_function_json();

}

// Settings
double libRR_playback_speed = 100;
string libRR_parse_message_from_web(json message_json) //string message)
{
  // printf("New Web Message %s \n", message.c_str());

  // auto message_json = json::parse(message);
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
    printf("Request for memory %s\n", message_json["state"]["memory"]["name"].dump(4).c_str());
    return get_memory_for_web(message_json["state"]["memory"]["name"], message_json["state"]["offset"], message_json["state"]["length"], message_json["state"]["swapEndian"]);
  }
  else if (category == "request_strings")
  {
    printf("Request for strings %s\n", message_json["state"]["name"].dump(4).c_str());
    return get_strings_for_web(message_json["state"]["memory"]["name"], message_json["state"]["offset"], message_json["state"]["length"]);
  }
  else if (category == "stop") {
    // retro_unload_game();
    retro_deinit();
    exit(0);
  }
  else if (category == "play") {
    printf("Got Play request from UI %s\n", message_json["state"].dump().c_str());

    int startAt = message_json["state"]["startAt"].get<int>();
    // First of all Load state if requested
    if (startAt == 0) {
      printf("Restart game\n");
      libRR_reset(0);
      retro_reset();
    }
    else if (startAt != -1) {
      printf("Load state: %d\n", startAt);
      // libRR_reset(0);
      libRR_load_save_state(startAt);
    }

    player_settings p2 = message_json["state"].get<player_settings>();
    libRR_settings = p2;
    libRR_full_function_log = p2.fullLogging;

    // Set the speed here
    libRR_playback_speed = message_json["state"]["speed"];
    printf("The speed will be %f \n", libRR_playback_speed);
    
    if (libRR_current_playthrough["last_frame"] != 0) {
      // std::cout << p2.dump(4) << std::endl;
      std::cout << "Would load:" << libRR_current_playthrough["current_state"]["frame"].dump(4) << std::endl;
      // libRR_load_save_state(libRR_current_playthrough["current_state"]["frame"]);
    }
    return "Running";
    //return game_json.dump(4);
  }
  else if (category == "pause") {
    printf("Pause request from UI %s\n", message_json["state"].dump().c_str());
    player_settings p2 = message_json["state"].get<player_settings>();
    libRR_settings = p2;
    libRR_full_function_log = p2.fullLogging;
    save_constant_metadata();
    libRR_export_all_files();
    return "Paused";
    // printf("Returning game_json dump (sometimes segfaults?) \n");
    // return game_json.dump(4);
  }
  else if (category == "restart") {
    libRR_reset(0);
    retro_reset();
  }
  else if (category == "save_state") {
    return libRR_create_save_state(message_json["state"]["name"], RRCurrentFrame);
  }
  else if (category == "delete_state") {
    return libRR_delete_save_state(message_json["state"]["frame"]);
  }
  else if (category == "change_input_buttons") {
    libRR_resave_button_state_to_file(current_playthrough_directory+"button_log.bin", -1, message_json["state"]["buttonChanges"]);
    return "Done";
  }
  else if (category == "load_state") {
    printf("WEB UI: Requested Load State\n");
    libRR_display_message("WEB UI: Requested Load State");
    libRR_reset(0);
    return libRR_load_save_state(message_json["state"]["frame"]);
  }
  else if (category == "modify_override") {
    printf("Add Code Override %s\n", message_json["state"].dump().c_str());
    string category = message_json["state"]["overrideType"];
    string name = message_json["state"]["name"];
    game_json["overrides"][category][name] = message_json["state"];
    saveJsonToFile(current_playthrough_directory+"/overrides.json", game_json["overrides"]);
  }
  else if (category == "modify_note") {
    printf("Add Note %s\n", message_json["state"].dump().c_str());
    string category = message_json["state"]["overrideType"];
    string name = message_json["state"]["name"];
    game_json["notes"][category][name] = message_json["state"];
    saveJsonToFile(libRR_project_directory+"/notes.json", game_json["notes"]);
  }
  else if (category == "export_function") {
    printf("Export Function %s\n", message_json["state"].dump().c_str());
    libRR_export_all_files();
    return "Done";
  }
  else if (category == "edit_function") {
    // TODO: instead call: edit_function(message_json["state"]);
    printf("Edit Function %s\n", message_json["state"].dump().c_str());
    for (auto& it : functions) {
      if (it.second.func_offset == message_json["state"]["func_offset"]) {
        printf("Found function to update %s \n", message_json["state"]["func_offset"].dump().c_str());
        functions[it.first].func_name = message_json["state"]["func_name"];
        functions[it.first].export_path = message_json["state"]["export_path"];
        // functions[it.first].additional = message_json["state"]["additional"];
        break;
      }
    }
    game_json["functions"] = functions;
    printf("Saving functions.json \n");
    saveJsonToFile(libRR_project_directory+"/functions.json", game_json["functions"]);
    return "Saved";
  }
  else if (category == "upload_linker_map") {
    upload_linker_map(message_json["state"]);
    saveJsonToFile(libRR_project_directory+"/linker_map.json", message_json["state"]);
    return "Uploaded linker map";
  }
  else if (category == "game_information") {
    printf("get game information \n");
  }
  else
  {
    printf("Unknown category %s with state: %s\n", category.c_str(), message_json["state"].dump().c_str());
  }

  libRR_display_message("Category: %s", category.c_str());

  // Update game_json based on emulator settings
  game_json["current_state"] = current_state;
  printf("About to set playthrough\n");
  game_json["playthrough"] = libRR_current_playthrough;
  game_json["cd_tracks"] = libRR_cd_tracks;
  printf("About to set functions\n");
  std::cout << "function map size is " << functions.size() << '\n';
  game_json["functions"] = functions;
  // cout << game_json["functions"].dump() << std::endl;
  printf("About to set function_usage\n");
  // cout << "playthorugh function usage:" << playthough_function_usage.dump() << "\n";
  game_json["function_usage"] = playthough_function_usage;
  printf("About to set functions_playthrough\n");
  // game_json["functions_playthough"] = function_playthough_info;
  printf("About to set assembly\n");
  // TODO: only send assembly when requested not on every load
  // game_json["assembly"] = libRR_disassembly;
  printf("About to set console specific json\n");
  add_console_specific_game_json();
  printf("About to return dump to client\n");
  
  return game_json.dump();

}
