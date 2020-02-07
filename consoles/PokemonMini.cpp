#include "../cdl/CDL.hpp"
#include "../../libretro/libretro.h"
#include <queue>
#include "../civetweb/include/civetweb.h"

// useful functions when working with libretro:
// * retro_get_memory_data
// * retro_get_memory_size

int   l_CurrentFrame;
int RRCurrentFrame=0;

/* Server context handle */
  struct mg_context *ctx;

void write_rom_mapping() {
    // save_cdl_files();
    printf("ROM_PARAMS.headername: %s \n", rom_name.c_str());
    string filename = "./configs/";
    filename+=rom_name;
    filename += ".config.yaml";
    ofstream file(filename, std::ios_base::binary);
    file << "# ROM splitter configuration file\n";
    file << "name: \"";
    file << rom_name;
    file << "\"\n";
    file <<"\n# base filename used for outputs - (please, no spaces)\n";
    file <<"basename: \"";
    file << rom_name;
    file << "\"\n";
    file <<"ranges:\n";
    file <<"  # start,  end,      type,     label\n";
    // file <<"  - [0x000000, 0x000040, \"header\", \"header\"]\n";   

    //
    // Write out 
    //
    for (auto& it : dmas) {
        auto t = it.second;
        if (it.first ==0 || t.dram_start == 0) continue;
        // file << create_n64_split_regions(t) << "\n";
    }

    file <<"# Labels for functions or data memory addresses\n";
    file <<"# All label addresses are RAM addresses\n";
    file <<"# Order does not matter\n";
    file <<"labels:\n";
    uint32_t entryPoint = 0x00; //ROM_HEADER.PC; // (int8_t*)(void*)&ROM_HEADER.PC;
    file << "   - [0x" << std::hex << __builtin_bswap32(entryPoint)+0 <<", \"EntryPoint\"]\n";
    for (auto& it : labels) {
        auto t = it.second;
        if (strcmp(t.func_offset.c_str(), "") == 0) continue;
        file << "   - [0x" << t.func_offset <<", \"" <<  t.func_name << "\"]\n";
    }

}

int
FileHandler(struct mg_connection *conn, void *cbdata)
{
	/* In this handler, we ignore the req_info and send the file "fileName". */
	const char *fileName = (const char *)cbdata;
  // cout << "Filename:" << fileName;
	mg_send_file(conn, "./libRetroReversing/websrc/index.html");
	return 1;
}

static int
handler(struct mg_connection *conn, void *ignored)
{
	const char *msg = "Hello world";
	unsigned long len = (unsigned long)strlen(msg);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	mg_write(conn, msg, len);
  printf("Connection made!");

	return 200;
}

int
log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}

struct mg_callbacks callbacks;
void setup_web_server() {
  printf("Setting up web server \n");
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.log_message = log_message;
   /* Initialize the library */
    mg_init_library(0);
    const char *options[] = {
      "document_root",
		"./libRetroReversing/websrc/",
		"listening_ports",
		"1234",
		"request_timeout_ms",
		"10000",
		"error_log_file",
		"error.log",
    "enable_directory_listing",
    "yes",
    0
	};

    /* Start the server */
    ctx = mg_start(&callbacks, 0, options);

    /* Add some handler */
    // mg_set_request_handler(ctx, "/test", FileHandler, 0);
}

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

struct retro_input_descriptor desc[9];
int total_input_buttons=0;
std::queue<unsigned long long> button_history;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total) {
  // desc = descriptor;
  total_input_buttons = total;
  // desc = (retro_input_descriptor*) malloc(sizeof(descriptor) * total);
  // memcpy(desc, descriptor, sizeof(descriptor));
  // memcpy(desc, descriptor, sizeof(desc));
  for (int i=0; i<total_input_buttons; i++) {
    // Copy libretro input descriptors to our own state
    desc[i] = { descriptor[i].port, descriptor[i].device, descriptor[i].index, descriptor[i].id,   descriptor[i].description };
  }
}

void save_button_state_to_file() {
  std::fstream output_file;
  output_file = std::fstream("button_log.bin", std::ios::out | std::ios::binary);

  while (!button_history.empty()) { 
      std::cout << ' ' << button_history.front(); 
      unsigned long long button_state = button_history.front();
      output_file.write(reinterpret_cast<char*>(&button_state),sizeof(unsigned long long));
      button_history.pop(); 
  } 
  output_file.close();

}
void log_input_state(retro_input_state_t input_cb) {
  unsigned long long frameInputBitField = 0;
  for (int i=0; i<total_input_buttons; i++) {
    // printf("Logging button number: %d %d \n", i, desc[i].id);
    if (input_cb(desc[i].port, desc[i].device, desc[i].index, desc[i].id) != 0) {
      frameInputBitField |= 1ULL << i;
    }
  }
  button_history.push(frameInputBitField);
  // printf("Logging input state frame:%d result:%d \n", RRCurrentFrame, frameInputBitField);
}


void read_button_state_from_file() {
  std::ifstream myfile("button_log_backup.bin", std::ios_base::in | std::ios::binary);
  unsigned long long frameInputBitField = 255;
  // myfile >> frameInputBitField;
  while (myfile.read(reinterpret_cast<char*>(&frameInputBitField), sizeof(unsigned long long)))
  {
    std::cout << ' ' << frameInputBitField; 
    button_history.push(frameInputBitField);
  }
  printf("Finished Reading input state frame:%d result:%d \n", RRCurrentFrame, frameInputBitField);

}

unsigned long long libRR_playback_next_input_state() {
  unsigned long long button_state = button_history.front();
  button_history.pop(); 
  return button_state;
}

void libRR_handle_load_game() {
  printf("Loading a new ROM \n");
  setup_web_server();
  read_button_state_from_file();
}

void handle_emulator_close() {
  save_button_state_to_file();
  mg_stop(ctx);
  mg_exit_library();
}

}
