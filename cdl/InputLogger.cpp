#include "CDL.hpp"
#include <queue>
#include "../include/libRR.h"

struct retro_input_descriptor desc[64];
int total_input_buttons=0;
std::queue<unsigned long long> button_history;
std::queue<unsigned long long> playback_button_history;
// Should append is for when you were playing back a history and it reached the end and then you added more input
bool libRR_should_append_history = false;

void libRR_setInputDescriptor(struct retro_input_descriptor* descriptor, int total) {
  // printf("libRR_setInputDescriptor \n");
  total_input_buttons = total;

  for (int i=0; i<total_input_buttons; i++) {
    // Copy libretro input descriptors to our own state
    desc[i] = { descriptor[i].port, descriptor[i].device, descriptor[i].index, descriptor[i].id,   descriptor[i].description };
  }
}

bool libRR_alreadyWarnedAboutEndOfLog = false;
// 
// playback_fake_input_state_cb - plays back input
// 
int lastPlayedBackFrame = 0;
int16_t playback_fake_input_state_cb(unsigned port, unsigned device,
      unsigned index, unsigned id) {
      // printf("playback_fake_input_state_cb\n");
      if(port > 0) {
        // printf("We only support Port 0 (player 1)\n");
        return 0;
      }

      if (playback_button_history.empty()) {
        if (!libRR_alreadyWarnedAboutEndOfLog) {
          printf("WARNING: button history was empty: probably at the end\n");
          libRR_alreadyWarnedAboutEndOfLog = true;
        }
        libRR_should_append_history = true;
        libRR_should_playback_input = false;
        return 0;
      }

      // This can be called multiple times per frame, so we need to only pop it when the frame has changed
      int16_t button_state = playback_button_history.front();
      if (RRCurrentFrame > lastPlayedBackFrame) {
        playback_button_history.pop();
        if (button_state>0) {
          libRR_display_message("Playback Pressed: %d",button_state);
        }
        lastPlayedBackFrame = RRCurrentFrame;
      }

      if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
        return button_state;
      }

      return button_state & 1 << id;
}

retro_input_state_t libRR_playback() {
  return playback_fake_input_state_cb;
}

// 
// libRR_playback_next_input_state - only use when you can't use playback_fake_input_state_cb due to no bitmask support
// 
unsigned long long libRR_playback_next_input_state() {
  if (playback_button_history.empty()) {
    printf("WARNING: button history was empty: probably at the end\n");
    libRR_should_append_history = true;
    return 0;
  }
  unsigned long long button_state = playback_button_history.front();
  playback_button_history.pop(); 
  std::cout << "\nPlayed back:" << button_state;
  return button_state;
}

// 
// # libRR_log_input_state_bitmask - this is the prefered solution to use in a core if possible
// 
void libRR_log_input_state_bitmask(retro_input_state_t input_cb) {
  int16_t ret = input_cb( 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK );
  button_history.push(ret);
  printf("Logging input state frame:%d result:%d \n", RRCurrentFrame, ret);
}

retro_input_state_t libRR_handle_input(retro_input_state_t input_cb) {
  if (libRR_should_playback_input) {
		return libRR_playback();
	}  
  libRR_log_input_state_bitmask(input_cb);
  return input_cb;
}


// 
// # log_input_state - only use this if you can't use libRR_log_input_state_bitmask
// 
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

// max_number is used if you want to only save up to a particular frame number
void libRR_resave_button_state_to_file(string filename, int max_number) {
  std::fstream output_file;
  // read the state before we open it as an output file
  libRR_read_button_state_from_file(filename);
  output_file = std::fstream(filename, std::ios::out | std::ios::binary);
  printf("libRR_resave_button_state_to_file max_number: %d\n", max_number);
  int frame_number = 0;
  while (!playback_button_history.empty()) { 
    unsigned long long button_state = playback_button_history.front();
    output_file.write(reinterpret_cast<char*>(&button_state),sizeof(unsigned long long));
    playback_button_history.pop(); 
    if (frame_number == max_number) {
      printf("Resaving button log, found Max value %d \n", frame_number);
      break;
    }
    frame_number++;
  } 
  output_file.close();
  libRR_read_button_state_from_file(filename);
}

// 
// # libRR_save_button_state_to_file - save all the keys pressed to a file
// 
void libRR_save_button_state_to_file(string filename) {
  std::fstream output_file;
  // read the state before we open it as an output file
  libRR_read_button_state_from_file(filename);
  output_file = std::fstream(filename, std::ios::out | std::ios::binary);
  printf("libRR_save_button_state_to_file\n");

  if (libRR_should_append_history) {
    // Use was playing back a history and now added additional logging
    printf("\n\n\nAppending history to previous\n\n\n");
    // libRR_read_button_state_from_file(filename);
    while (!playback_button_history.empty()) { 
      unsigned long long button_state = playback_button_history.front();
      output_file.write(reinterpret_cast<char*>(&button_state),sizeof(unsigned long long));
      playback_button_history.pop(); 
    } 

  }

  while (!button_history.empty()) { 
      // std::cout << ' ' << button_history.front(); 
      unsigned long long button_state = button_history.front();
      output_file.write(reinterpret_cast<char*>(&button_state),sizeof(unsigned long long));
      button_history.pop(); 
  } 
  output_file.close();

}

// 
// # libRR_read_button_state_from_file - loads the buffer of input key presses to be run each frame
//
void libRR_read_button_state_from_file(string filename, int start_frame) {
  std::ifstream myfile(filename, std::ios_base::in | std::ios::binary);
  unsigned long long frameInputBitField = 255;
  lastPlayedBackFrame = 0;
  int loading_frame = 0;
  playback_button_history = {};
  while (myfile.read(reinterpret_cast<char*>(&frameInputBitField), sizeof(unsigned long long)))
  {
    if (loading_frame >= start_frame) {
      std::cout << ' ' << frameInputBitField; 
      playback_button_history.push(frameInputBitField);
    }
    loading_frame++;
  }
  printf("Finished Reading input state frame:%d size:%d \n", start_frame, playback_button_history.size());

}