#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"
#include <cstdarg>

#ifndef EMSCRIPTEN
// TODO: check if this include can be removed since we are already including it in CDL.hpp
#endif

// Variables
char libRR_save_directory[4096];
libRR_frame_buffer libRR_current_frame_buffer = {};
unsigned int libRR_current_frame_buffer_length = 0;
bool libRR_should_playback_input = true;
bool libRR_should_log_input = false;
int libRR_last_logged_frame = 0;
string current_playthrough_directory = "";
json game_json = json::object();
json override_code_json = {};
json playthroughs_json = {};
json libRR_current_playthrough = {};
json playthough_function_usage = {};
json libRR_console_constants = {};
string libRR_project_directory = "";
string libRR_export_directory = "";
string libRR_current_playthrough_name = "Initial";
int libRR_should_Load_EPROM = 0;
int libRR_message_duration_in_frames = 180;
json libRR_settings = json::parse("{ \"paused\": true, \"fullLogging\": false }");
extern bool libRR_full_trace_log;
extern retro_environment_t environ_cb;

std::map<string, libRR_emulator_state> playthroughs = {};
// current_emulator_state holds all the game core information such as Game Name, CD Tracks, Memory regions etc
// This is never changed from Web requests, and is not often changed at all
// /*libRR_emulator_state*/ json current_emulator_state = {};

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
#ifndef EMSCRIPTEN
  fs::create_directories( current_playthrough_directory );
#endif
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
    functions = game_json["functions"].get<std::map<uint32_t, cdl_labels>  >();
  }

  cout << "About to save playthough metadata" << std::endl;
  save_playthough_metadata();
  cout << "About to save constant metadata" << std::endl;
  save_constant_metadata();
  cout << "About to read button state to memory" << std::endl;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", 0);

  if (!libRR_current_playthrough["last_frame"].is_null()) {
    libRR_last_logged_frame = libRR_current_playthrough["last_frame"];
  } else {
    printf("Current Playthrough's last frame is null /n");
  }
  libRR_should_playback_input = true;
  printf("Loaded last logged frame: %d\n",libRR_last_logged_frame);
}

void libRR_define_console_memory_region(string name, unsigned long long start, unsigned long long end, long long mirror_address)
{
  cout << name << "\n";
}

json libRR_get_list_of_memory_regions()
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
  return memory_descriptors;
}

void libRR_setup_retro_base_directory(retro_environment_t _environ_cb) {
  // Setup path
  const char *dir = NULL;

  if (_environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir) {
        snprintf(libRR_save_directory, sizeof(libRR_save_directory), "%s", dir);
    }
  else {
      snprintf(libRR_save_directory, sizeof(libRR_save_directory), "%s", ".");
  }
  dir = NULL; 

   if (_environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
   {
      snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
   }
   else {
     snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", libRR_save_directory);
   }
  // end setup path
}

void libRR_setup_directories(retro_environment_t _environ_cb) {
  printf("libRR_setup_directories");
  libRR_setup_retro_base_directory(_environ_cb);

#ifdef EMSCRIPTEN
  libRR_project_directory = "";
#else
  libRR_project_directory = retro_base_directory;
#endif
  libRR_project_directory += "/RE_projects/";
  libRR_project_directory += libRR_console; //current_state.libretro_system_info.library_name;
  libRR_project_directory += "/" + libRR_game_name + "/";
  libRR_export_directory += libRR_project_directory + "src/";
  #ifndef EMSCRIPTEN
  fs::create_directories( libRR_project_directory);
  fs::create_directories( libRR_project_directory + "/playthroughs/");
  fs::create_directories( libRR_export_directory);
  #endif
  cout << "Created project directory: " << libRR_project_directory << std::endl;
}

// 
// # Read all JSON config
// 
void read_json_config() {
  cout << "Project directory:" << libRR_project_directory << std::endl;
  // readJsonToObject(libRR_project_directory+"/game.json", game_json);
  // cout << game_json.dump(4) << std::endl;
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
extern string libRR_rom_name;
void libRR_handle_load_game(const struct retro_game_info *info, retro_environment_t _environ_cb)
{
  printf("Loading a new ROM \n");
  libRR_setup_console_details(_environ_cb);

  libRR_rom_name = extract_basename(info->path);

  printf("Game path: %s name: %s\n", info->path, libRR_rom_name.c_str());

  libRR_game_name = alphabetic_only_name((char*)libRR_rom_name.c_str(), libRR_rom_name.length());

  RRCurrentFrame = 0;

  // 
  // Setup reversing files
  // 
  libRR_setup_directories(_environ_cb);
  read_json_config();
  init_playthrough(libRR_current_playthrough_name); // todo get name from front end
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

  cout << "About to save trace log (flush)";
  libRR_log_trace_flush();
  game_json["functions"] = functions;
  saveJsonToFile(libRR_project_directory+"/functions.json", game_json["functions"]);
}

void save_playthough_metadata() {
  printf("Save Playthough Meta Data");
  if (libRR_current_playthrough.count("name") < 1) {
    printf("libRR_current_playthrough does not have a name so lets create a new one");
    libRR_current_playthrough["name"] = libRR_current_playthrough_name;
    libRR_current_playthrough["states"] =  json::parse("[]");
    // libRR_current_playthrough["current_save_state"] =  json::parse("{}");
    libRR_current_playthrough["last_frame"] =  0;
  }
  saveJsonToFile(current_playthrough_directory+"/playthrough.json", libRR_current_playthrough);
  saveJsonToFile(current_playthrough_directory+"/resources.json", game_json["cd_data"]["root_files"]);
  saveJsonToFile(current_playthrough_directory+"/overrides.json", game_json["overrides"]);
  saveJsonToFile(current_playthrough_directory+"/function_usage.json", playthough_function_usage);
}

void libRR_reset(unsigned int reset_frame) {
  printf("libRR_reset to frame: %d\n", reset_frame);
  RRCurrentFrame = reset_frame;
  libRR_should_playback_input = true;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", reset_frame);
}

string libRR_load_save_state(int frame) {
  printf("libRR_load_save_state frame: %d \n", frame);
  size_t length_of_save_buffer = retro_serialize_size();
  uint8_t *data = (uint8_t *)malloc(length_of_save_buffer);
  string filename = "save_"+to_string(frame)+".sav";
  libRR_read_binary_data_from_file(data, length_of_save_buffer, current_playthrough_directory+filename);
  libRR_direct_unserialize(data, length_of_save_buffer);
  RRCurrentFrame = frame;
  libRR_should_playback_input = true;
  libRR_read_button_state_from_file(current_playthrough_directory+"button_log.bin", frame);
  printf("End of libRR_load_save_state at frame: %d\n", frame);
  return libRR_current_playthrough.dump(4);
} 

void libRR_load_save_state_c(int frame) {
  printf("Called C version of libRR_load_save_state");
  libRR_load_save_state(frame);
}

string libRR_delete_save_state(int frame) {
  printf("libRR_delete_save_state frame: %d\n", frame);
  string filename = current_playthrough_directory+"save_"+to_string(frame)+".sav";
  string png_filename = filename+".png";
  libRR_delete_file(filename);
  libRR_delete_file(png_filename);

  json j = libRR_current_playthrough["states"];
  int i=0;

// Loop over frames and delete the one we don't want
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    json current = *it;
    if (current["frame"] == frame) {
      printf("Found the frame: %d\n", frame);
      // We found the frame in the list so now Delete this frame
      current["frame"] = -1; // set it to -1 just incase
      libRR_current_playthrough["states"].erase(i); // now remove it from the list
      break;
    }
    i++;
  }


  json next_latest_state; // store the next highest state after this one
  int highest_frame = 0;
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    json current = *it;
    printf("Loop over all frames current: %d\n", (int) current["frame"]);
    if (current["frame"] >= highest_frame) {
      highest_frame = current["frame"];
    }
    if (next_latest_state.is_null() || current["frame"]>next_latest_state["frame"]) {
      // if the one we are deleting is latest then we need to find the next latest
      next_latest_state = current;
    }
  }

  // int latest_state_number = libRR_current_playthrough["current_state"]["frame"];
  if (frame >= highest_frame && !next_latest_state.is_null()) {
      printf("Deleting most recent state (state with highest frame number)\n");
      // User is deleting the last known state so we need special handling
      libRR_current_playthrough["current_state"] = next_latest_state;
      libRR_current_playthrough["last_frame"] = next_latest_state["frame"];
      printf("Since we are deleting the latest state, we will go back to: %d \n", (int)next_latest_state["frame"]);
      // next we want to remove some entries from the button log
      libRR_resave_button_state_to_file(current_playthrough_directory+"button_log.bin", (int)next_latest_state["frame"]);
  } else {
    printf("We are not deleting the latest state so we will not modify the button state\n");
  }

  save_playthough_metadata();
  return libRR_current_playthrough.dump(4);
}

string libRR_create_save_state(string name, unsigned int frame, bool fast_save = false) {

  string filename = "save_"+to_string(frame)+".sav";

  // Create Save state
  size_t length_of_save_buffer = retro_serialize_size();
  uint8_t *data = (uint8_t *)malloc(length_of_save_buffer);
  libRR_direct_serialize(data, length_of_save_buffer);
  cout << "libRR_create_save_state Length of save buffer: " << length_of_save_buffer << " Name from client:" << name << " At Frame:" << frame << std::endl;
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
  libRR_current_playthrough["current_save_state"] = state;

  if (RRCurrentFrame > libRR_current_playthrough["last_frame"]) {
    printf("RRCurrentFrame is greater than last_frame of Playthrough so setting Last frame to: %d", RRCurrentFrame);
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

void libRR_create_save_state_c(const char* name, int frame, bool fast_save) {
  printf("Called C version of libRR_create_save_state");
  libRR_create_save_state(name, frame);
}

string libRR_get_data_for_file(int offset, int length, bool swapEndian);


uint8_t* get_memory_pointer(string memory_name, int offset, int length) {
  if (memory_name == "file") {
    return NULL;
    // return libRR_get_data_for_file(offset, length);
  }
  json memory_descriptors = libRR_get_list_of_memory_regions();
  for (auto &i : memory_descriptors)
  {
    string address_space_name = i["addrspace"];
    if (address_space_name == memory_name)
    {
      int start_offset = i["start"];
      int length_of_memory = i["len"];
      int pointer_address = i["ptr"];
      int end = start_offset + length_of_memory;
      if (start_offset + offset >= end)
      {
        // Starting at the end is no good
        return NULL;
      }
      if ((start_offset + offset + length) >= end)
      {
        length = end - (start_offset + offset);
      }
      return (uint8_t *)(pointer_address) + offset;
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
  json memory_descriptors = libRR_get_list_of_memory_regions();
  for (auto &i : memory_descriptors)
  {
    int start_offset = i["start"];
    int length_of_memory = i["len"];
    int pointer_address = i["ptr"];
    int end = start_offset + length_of_memory;
    if (offset >= start_offset && offset < end) {
      int relative_offset = offset - start_offset;
      if ((offset + length) >= end)
      {
        length = end - (offset);
      }
      
      if (asHexString) {
        return printBytesToStr((uint8_t *)(pointer_address) + relative_offset, length, swapEndian);
      }
      // printf("Found Name: %s start: %d end: %d \n", i.addrspace, i.start, end);
      return printBytesToDecimalJSArray((uint8_t *)(pointer_address) + relative_offset, length, swapEndian);
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
  json memory_descriptors = libRR_get_list_of_memory_regions();
  for (auto &i : memory_descriptors)
  {
    string memory_descriptor_name = i["name"];
    if (memory_descriptor_name == memory_name)
    {
      int start_offset = i["start"];
      int length_of_memory = i["length"];
      int end = start_offset + length_of_memory;
      if (start_offset + offset >= end)
      {
        // Starting at the end is no good
        return "[]";
      }
      if ((start_offset + offset + length) >= end)
      {
        length = end - (start_offset + offset);
      }
      int pointer_address = i["pointer"];
      return printBytesToDecimalJSArray((uint8_t *)(pointer_address) + offset, length, swapEndian);
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

  if (environ_cb != NULL)
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


string message_result = ""; // Store message_result on heap
__attribute__((export_name("libRR_parse_message_from_emscripten"))) const char* libRR_parse_message_from_emscripten(const char* json_message) {
  printf("libRR_parse_message_from_emscripten %s \n", json_message);
  libRR_full_trace_log = false;
  if (json::accept(json_message)) {
    message_result =  libRR_parse_message_from_web(json::parse(json_message));
    return message_result.c_str();
  }
  printf("ERROR in libRR_parse_message_from_emscripten, this is not valid JSON: %s \n", json_message);
  return "Error invalid JSON provided to libRR_parse_message_from_emscripten";
}

string dump_to_return = "";
json result_json = {};
string return_json_to_web(json result) {
  result_json["result"] = result;
  dump_to_return = result_json.dump(1, ' ', true, nlohmann::detail::error_handler_t::replace);
  printf("About to return dump to web client\n");
  return dump_to_return;
}

// Settings
double libRR_playback_speed = 100;
string libRR_parse_message_from_web(json message_json) //string message)
{
  printf("New Web Message %s \n", message_json.dump().c_str());

   if (!message_json.contains("category")) {
     return "No category defined";
   }

  // auto message_json = json::parse(message);
  string category = message_json["category"].get<std::string>();
  
  // if (category == "player_settings")
  // {
  //   printf("OLD Player settings!\n");
  //   return game_json.dump(4);
  // }
  // else 
  if (category == "request_memory")
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

    libRR_settings = message_json["state"];
    libRR_full_function_log = libRR_settings["fullLogging"];
    libRR_full_trace_log = libRR_full_function_log;

    // Set the speed here
    libRR_playback_speed = message_json["state"]["speed"];
    printf("The speed will be %f \n", libRR_playback_speed);
    
    if (libRR_current_playthrough["last_frame"] != 0) {
      // std::cout << p2.dump(4) << std::endl;
      std::cout << "Would load:" << libRR_current_playthrough["current_save_state"]["frame"].dump(4) << std::endl;
      // libRR_load_save_state(libRR_current_playthrough["current_state"]["frame"]);
    }
    return "Running";
    //return game_json.dump(4);
  }
  else if (category == "pause") {
    printf("Pause request from UI %s\n", message_json["state"].dump().c_str());
    libRR_settings = message_json["state"];
    libRR_full_function_log = libRR_settings["fullLogging"];
    save_constant_metadata();
    libRR_export_all_files();
    return "Paused";
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
  else if (category == "emulator_metadata") {
    // printf("Get Emulator Meta Data (info only ever changed on backend) \n");
    json current_emulator_state = {};
    current_emulator_state["game_name"] = libRR_game_name;
    current_emulator_state["rom_name"] = libRR_rom_name;
    current_emulator_state["memory_descriptors"] = libRR_get_list_of_memory_regions();
    current_emulator_state["cd_tracks"] = libRR_cd_tracks;
    current_emulator_state["libRR_save_states"] = libRR_save_states;
    current_emulator_state["libRR_current_playthrough"] = libRR_current_playthrough;
    current_emulator_state["RRCurrentFrame"] = RRCurrentFrame;

    json paths = json::parse("{}");
    paths["libRR_project_directory"] = libRR_project_directory;
    paths["retro_save_directory"] = libRR_save_directory;
    paths["retro_base_directory"] = retro_base_directory;
    paths["retro_cd_base_directory"] = retro_cd_base_directory;
    paths["retro_cd_path"] = retro_cd_path;
    current_emulator_state["paths"] = paths;

    return return_json_to_web(current_emulator_state);
  }
  else
  {
    printf("Unknown category %s with state: %s\n", category.c_str(), message_json["state"].dump().c_str());
  }

  libRR_display_message("Category: %s", category.c_str());

  // Update game_json based on emulator settings
  // game_json["current_state"] = current_state;
  // printf("About to set playthrough\n");
  // game_json["playthrough"] = libRR_current_playthrough;
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
  printf("About to convert game_json dump to string\n");
  string dump = game_json.dump(1, ' ', true, nlohmann::detail::error_handler_t::replace);
  printf("About to return dump to client\n");
  
  return dump;
}
