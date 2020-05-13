#include "CDL.hpp"
#include <queue>
#include "../include/libRR.h"

int libRRshouldLogInput = 0;

struct retro_input_descriptor desc[9];
  int total_input_buttons=0;
  std::queue<unsigned long long> button_history;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total) {
  // desc = descriptor;
  total_input_buttons = total;

  for (int i=0; i<total_input_buttons; i++) {
    // Copy libretro input descriptors to our own state
    desc[i] = { descriptor[i].port, descriptor[i].device, descriptor[i].index, descriptor[i].id,   descriptor[i].description };
  }
}

unsigned long long libRR_playback_next_input_state() {
  unsigned long long button_state = button_history.front();
  button_history.pop(); 
  // std::cout << "\nPlayed back:" << button_state;
  return button_state;
}

void log_input_state(retro_input_state_t input_cb) {
  unsigned long long frameInputBitField = 0;
  for (int i=0; i<total_input_buttons; i++) {
    // printf("Logging button number: %d %d \n", i, desc[i].id);
    if (input_cb(desc[i].port, desc[i].device, desc[i].index, desc[i].id) != 0) {
      frameInputBitField |= 1ULL << desc[i].id;
    }
  }
  button_history.push(frameInputBitField);
  printf("Logging input state frame:%d result:%d \n", RRCurrentFrame, frameInputBitField);
}

void libRR_save_button_state_to_file() {
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

void libRR_read_button_state_from_file() {
  std::ifstream myfile("clean.bin", std::ios_base::in | std::ios::binary);
  unsigned long long frameInputBitField = 255;
  while (myfile.read(reinterpret_cast<char*>(&frameInputBitField), sizeof(unsigned long long)))
  {
    std::cout << ' ' << frameInputBitField; 
    button_history.push(frameInputBitField);
  }
  printf("Finished Reading input state frame:%d result:%d \n", RRCurrentFrame, frameInputBitField);

}