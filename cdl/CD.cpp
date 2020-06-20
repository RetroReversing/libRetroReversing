#include "../include/libRR.h"
#include "CDL.hpp"
#include <fstream>
#include <sstream>

extern json game_json;

#define USER_SECTOR_SIZE 2048
#define FULL_SECTOR_SIZE 2352
char* start_pointer;

json parse_root_files(char*& data, json pvd, uint32_t lba, uint32_t directory_size);

string readString(char*& data, int length) {
  string value = string(data,length);
  data = (char*)data+length;
  return value;
}

string readString(char*& data, int length, string assertion) {
  string value = string(data,length);
  printf("readString %s assert: %s \n", value.c_str(), assertion.c_str());
  if (value != assertion) {
    printf("ERROR: Assertion failed %s is supposed to equal %s\n", value.c_str(), assertion.c_str());
  }
  data = (char*)data+length;
  return value;
}

// read2ByteWordTwice is used because the ISO CD format store the length data in both Big Endian and Little endian, so total 4 bytes
uint16_t read2ByteWordTwice(char*& data) {
  uint16_t value = ((uint16_t*)data)[0]; // we only care about Little endian
  data = (char*)data+4; // add 4 to ignore big endian
  return value;
}
uint16_t read2ByteWordTwice(char*& data, int assertion) {
  uint16_t value = read2ByteWordTwice(data);
  if (value != assertion) {
    printf("ERROR: Assertion failed %d is supposed to equal %d\n", value, assertion);
  }
  return value;
}

uint32_t read4ByteWord(char*& data) {
  uint32_t value = ((uint32_t*)data)[0];
  data = (char*)data+4;
  return value;
}

uint32_t read4ByteWordTwice(char*& data) {
  uint32_t value = ((uint32_t*)data)[0]; // we only care about Little endian
  data = (char*)data+8; // add 8 to ignore big endian
  return value;
}
uint32_t read4ByteWordTwice(char*& data, int assertion, string name) {
  uint32_t value = read4ByteWordTwice(data);
  if (value != assertion) {
    printf("ERROR: %s Assertion failed %d is supposed to equal %d\n", name.c_str(), value, assertion);
  }
  return value;
}

uint16_t read2ByteWord(char*& data) {
  uint16_t value = ((uint16_t*)data)[0];
  data = (char*)data+2;
  return value;
}

char read1ByteTwice(char*& data) {
  char value = (char)data[0];
  data = (char*)data+2; //ignore big endian
  return value;
}
char read1Byte(char*& data) {
  char value = (char)data[0];
  data = (char*)data+1;
  return value;
}

void seek(char*& data, unsigned int location, string reason) {
  data += location;
}

json parse_7byte_datatime(char*& data) {
  json datetime;
  datetime["year"] = read1Byte(data);
  datetime["month"] = read1Byte(data);
  datetime["day"] = read1Byte(data);
  datetime["hour"] = read1Byte(data);
  datetime["minute"] = read1Byte(data);
  datetime["second"] = read1Byte(data);
  datetime["timezone"] = read1Byte(data);
  return datetime;
}

// 
// # Parse a directory entry (ISO 9660)
// 
json parse_directory_entry(char*& data, json pvd, string pvd_key_name) {
  json root_directory;
  int length = read1Byte(data);
  if (length == 0) {
    return NULL;
  }
  root_directory["length_extended_attribute_record"] = read1Byte(data);
  root_directory["extent_location_lba"] = read4ByteWordTwice(data);
  root_directory["extent_location_bytes"] = ((int)root_directory["extent_location_lba"]) * FULL_SECTOR_SIZE;
  root_directory["extent_length"] = read4ByteWordTwice(data);
  root_directory["extent_location_end_lba"] = ((int)root_directory["extent_location_lba"]) + (((int) root_directory["extent_length"])/USER_SECTOR_SIZE);
  root_directory["datetime"] = parse_7byte_datatime(data);
  root_directory["file_flags"] = read1Byte(data);
  root_directory["file_unit_size"] = read1Byte(data);
  root_directory["interleave_gap_size"] = read1Byte(data);
  root_directory["volume_sequence_number"] = read2ByteWordTwice(data);

  int len_dir_name = read1Byte(data);
  root_directory["name"] = readString(data,len_dir_name);
  string name = root_directory["name"];
  char first_character = name[0];
  
  if ((root_directory["file_flags"] == 2) && (first_character != 0) && (first_character != 1)) {
    printf("This is a directory %s \n", root_directory["name"].dump().c_str());
    
    char* backup_pointer = data; // backup pointer
    pvd[root_directory["name"].dump().c_str()] = parse_root_files(data, pvd, root_directory["extent_location_lba"],root_directory["extent_length"]); 
    root_directory["files"] = parse_root_files(data, pvd, root_directory["extent_location_lba"],root_directory["extent_length"]); 
    data = backup_pointer; // restore pointer
  }

  if (pvd_key_name == "") {
    return root_directory;
  }
  pvd[pvd_key_name] = root_directory;
  return pvd;
}

// 
// # Parse the PVD (ISO 9660)
// 
json parse_primary_volume_descriptor(char* data) {
  json pvd;
  char version = read1Byte(data);
  seek(data, 1, "Unused byte");
  string systemId = readString(data, 32, "SEGA SEGASATURN                 ");
  pvd["volumeId"] = readString(data, 32);
  seek(data, 8, "Unused bytes");
  pvd["length_of_volume"] = read4ByteWordTwice(data, 12551, "length_of_volume");
  seek(data, 32, "Unused bytes");
  pvd["volume_set_size"] = read2ByteWordTwice(data, 1);
  pvd["volume_sequence_size"] = read2ByteWordTwice(data, 1);
  pvd["logical_block_size"] = read2ByteWordTwice(data, USER_SECTOR_SIZE);
  pvd["path_table_size"] = read4ByteWordTwice(data, 64, "path_table_size");

  pvd["path_table_location_little_endian"] = read4ByteWord(data);
  pvd["path_table_location_optional_little_endian"] = read4ByteWord(data);
  pvd["path_table_location_big_endian"] = read4ByteWord(data);
  pvd["path_table_location_optional_big_endian"] = read4ByteWord(data);

  // Root directory
  pvd = parse_directory_entry(data, pvd, "root_directory");

  pvd["volume_set_id"] = readString(data, 128);
  pvd["publisher_id"] = readString(data, 128);
  pvd["data_preparer_id"] = readString(data, 128);
  pvd["application_id"] = readString(data, 128);

  pvd["copyright_file_id"] = readString(data, 37);
  pvd["abstract_file_id"] = readString(data, 37);
  pvd["bibliographic_file_id"] = readString(data, 37);

  pvd["volume_create_datetime"] = readString(data, 17);
  pvd["volume_modified_datetime"] = readString(data, 17);
  pvd["volume_expire_datetime"] = readString(data, 17);
  pvd["volume_effective_datetime"] = readString(data, 17);
  pvd["file_structure_version"] = read1Byte(data);
  seek(data, 1, "Unused byte");
  seek(data, 512, "application area");
  seek(data, 512, "Reserved for Future use");
  return pvd;
}

// 
// # Path entry are all the directories (ISO 9660)
// 
json parse_path_entry(char*& data) {
  json path_entry;
  int len_dir_name = read1Byte(data);
  if (len_dir_name == 0) {
    return NULL;
  }
  path_entry["len_ext_attr_rec"] = read1Byte(data);
  path_entry["lba_extent"] = read4ByteWord(data);
  path_entry["parent_dir_idx"] = read2ByteWord(data);
  path_entry["directory_name"] = readString(data,len_dir_name);
  if (len_dir_name % 2 == 1) {
    seek(data, 1, "Padding byte");
  }
  return path_entry;
}

// The Path table is just a list of directories on the CD
json parse_path_table(char*& data, json pvd, int path_table_size) {
  // int path_table_size = pvd["path_table_size"];
  
  int number_of_sectors_for_path_table = (path_table_size / USER_SECTOR_SIZE) + (path_table_size % USER_SECTOR_SIZE ? 1 : 0);

  for (int sector = 0; sector < number_of_sectors_for_path_table; sector++) {
    int location_of_sector = ((int)pvd["path_table_location_little_endian"]) + sector;
    printf("sector: %d location_of_sector: %d number_of_sectors_for_path_table %d \n", sector, location_of_sector, number_of_sectors_for_path_table);
    
    data = start_pointer; // reset data pointer to start of file
    seek(data, location_of_sector * FULL_SECTOR_SIZE, "Go to location of sector");
    seek(data, 16, "Ignore Sector Header");

    for(int i =0; i<9999; i++) {
      json path_entry = parse_path_entry(data);
      if (path_entry == NULL) {
        break;
      }
      pvd["path_entry"].push_back(path_entry);
    }

  }
  return pvd;
}


json parse_root_files(char*& data, json pvd, uint32_t lba, uint32_t directory_size) {
  json files;
  int number_of_sectors_for_directory = (directory_size / USER_SECTOR_SIZE);

  for (int sector = 0; sector < number_of_sectors_for_directory; sector++) {
    int location_of_sector = lba + sector;
    // printf("number_of_sectors_for_directory: %d sector: %d location_of_sector: %d \n", number_of_sectors_for_directory, sector, location_of_sector);
    data = start_pointer; // reset data pointer to start of file
    seek(data, location_of_sector * FULL_SECTOR_SIZE, "Go to location of sector");
    seek(data, 16, "Ignore Sector Header");
    char* end_of_sector = (data)+USER_SECTOR_SIZE;

    for(int i =0; i<9999; i++) {
      json path_entry;
      if (data >= end_of_sector) {
        printf("Data: %d End of Sector: %d \n", data, end_of_sector);
        break;
      }
      path_entry = parse_directory_entry(data, path_entry, "");
      if (path_entry == NULL) {
        continue;
      }
      files.push_back(path_entry);
    }
  }
  return files;
}
// 
// # Parse Sega Saturn CD Track
// returns whether this is a valid ISO 9660 Track, otherwise its probably audio
// 
bool parse_sega_saturn(char* data, unsigned int data_length ) {
  start_pointer = &data[0];
  printf("parse_sega_saturn\n");
  seek(data, 0x9310, "Ignore Boot rom data");
  char value = read1Byte(data);
  string magic = readString(data, 5, "CD001");
  if (magic != "CD001") {
    return false; // probably an audio track so ignore
  }

  json pvd = parse_primary_volume_descriptor(data);
  
  pvd = parse_path_table(data, pvd, pvd["path_table_size"]);

  // Now lets get top level files
  uint32_t lba = pvd["root_directory"]["extent_location_lba"];
  uint32_t directory_size = pvd["root_directory"]["extent_length"];

  string dump = game_json["cd_data"]["root_files"].dump();
  if (game_json.count("cd_data") != 0 && game_json["cd_data"].count("root_files") != 0 && dump != "null" && dump != "{}") {
    // if we already have the layout of the CD from a previous run then we don't need to re-read it
    printf("No need to parse CD layout, reading from previous run \n");
    pvd["root_files"] = game_json["cd_data"]["root_files"];
  }
  else {
    pvd["root_files"] = parse_root_files(data, pvd, lba, directory_size);
    printf("pvd %s\n",pvd.dump(4).c_str());
  }
  game_json["cd_data"] = pvd;
  return true;
}

std::vector<libRR_cd_track> libRR_cd_tracks;
void libRR_add_cd_track(string name, void* data, unsigned int data_length) {
  printf("\n\nlibRR_add_cd_track %s \n\n",name.c_str());
  libRR_cd_track track = {};
  track.data = data;
  track.length = data_length;
  track.name = name;
  track.isData = parse_sega_saturn((char*)data,data_length );
  libRR_cd_tracks.push_back(track);
}

void libRR_replace_lba_buffer(int lba) {
  printf("libRR_replace_lba_buffer %d \n", lba);
  uint8_t* buf = (uint8_t*)libRR_get_current_buffer();
  for (auto track : libRR_cd_tracks) {
    if (!track.isData) { continue; }

    int location_of_sector = (FULL_SECTOR_SIZE * lba); //+ 16;
    memcpy((uint8_t *)buf, ((char*)track.data)+location_of_sector, FULL_SECTOR_SIZE); //USER_SECTOR_SIZE+100);
    // }
    // printf("Just about to print bytes to decimal \n");
    // result = printBytesToDecimalJSArray((uint8_t*)buffer, length);
  }
}

string libRR_get_data_for_file(int offset, int length) {
  printf("libRR_get_data_for_file %d length: %d \n", offset, length);
  int number_of_sectors_for_file = (length / USER_SECTOR_SIZE);
  if (length % USER_SECTOR_SIZE > 0) {
    number_of_sectors_for_file+=1;
  }
  printf("number_of_sectors_for_file %d \n", number_of_sectors_for_file);
  string result = "";
  unsigned int full_size = number_of_sectors_for_file*FULL_SECTOR_SIZE;
  char* buffer = (char*)malloc (sizeof(char) * (full_size+1));
  printf("Created Buffer successful %d \n", full_size);

  for (auto track : libRR_cd_tracks) {
    if (!track.isData) { continue; }
    for (int sector = 0; sector < number_of_sectors_for_file; sector++) {
      int sector_offset = (USER_SECTOR_SIZE * sector);
      int location_of_sector = offset + (FULL_SECTOR_SIZE * sector) +16;
      memcpy(buffer+sector_offset, ((char*)track.data)+location_of_sector, USER_SECTOR_SIZE);
    }
    printf("Just about to print bytes to decimal \n");
    result = printBytesToDecimalJSArray((uint8_t*)buffer, length);
  }
  printf("Just about to free buffer \n");
  free(buffer);

  return result;
}

bool libbRR_cd_can_log = false;
void libRR_cd_set_able_to_log(bool enable) {
  libbRR_cd_can_log = enable;
}
bool libRR_enable_overrides = false;
void libRR_cd_set_able_override(bool enable) {
  libRR_enable_overrides = enable;
}

// This is used if you have an lba and want to search while file name it is
string libRR_check_which_file_has_lba(int32_t lba, json files) {
  int index = 0;
  string result = "";
  for (auto& a: files) {
    int file_flags = a["file_flags"];
    if (file_flags == 2) {
      result = libRR_check_which_file_has_lba(lba, a["files"]);
      continue;
    }
    int extent_location_lba = a["extent_location_lba"];
    int extent_location_end_lba = a["extent_location_end_lba"];
    if (lba >= extent_location_lba && lba <= extent_location_end_lba) {
      printf("Found it: %s lba: %d \n", a["name"].dump().c_str(), lba);
      result= result + (string)a["name"];
    }
    index++;
  }
  return result;
}

json log_access_of_file_by_lba(int32_t lba, json& files) {
  int index = 0;
  for (auto& a: files) {
    int file_flags = a["file_flags"];
    if (file_flags == 2) {
      json result = log_access_of_file_by_lba(lba, a["files"]);
      if (result != NULL) {
        return files;
      }
      continue;
    }
    int extent_location_lba = a["extent_location_lba"];
    int extent_location_end_lba = a["extent_location_end_lba"];
    if (lba >= extent_location_lba && lba <= extent_location_end_lba) {
      if (a.count("first_access") == 0) {
        a["first_access"] = RRCurrentFrame;
        a["last_access"] = RRCurrentFrame;
        a["access_count"] = 1;
      } else {
          int last_access = a["last_access"];
          if (last_access < RRCurrentFrame) {
            a["last_access"] = RRCurrentFrame;
          }
          a["access_count"] = ((int)a["access_count"]) + 1;
      }
      return files;
    }
    index++;
  }
  return NULL;
}


int libRR_current_lba = 0;
void* libRR_current_buffer = 0;

int libRR_get_current_lba() {
  return libRR_current_lba;
}

void* libRR_get_current_buffer() {
  return libRR_current_buffer;
}

void libRR_memset(int startOffset, int length, uint8_t value ) {
  uint8_t* buf = (uint8_t*)libRR_get_current_buffer();
  printf("libRR_memset offset: %d length: %d value: %d \n", startOffset, length, value);
  memset(buf+startOffset, value, length);
  printf("done.\n");
}

void libRR_log_cd_access(int32_t lba) {
  if (!libbRR_cd_can_log) return;
  json& root_files =  game_json["cd_data"]["root_files"];
  json modified_root_files = log_access_of_file_by_lba(lba, root_files);
  
  // Find custom:
  // libRR_check_which_file_has_lba(7414, root_files);
}

void libRR_override_cd_lba(uint8_t *buf, int32_t lba, int mode) {
  if (!libRR_enable_overrides) {
    return;
  }
  for (auto a : game_json["overrides"]["CD"]) {
    if (!a["enabled"]) {
      continue;
    }
    if (lba >= a["start"] && lba <= a["end"]) {
      // printf("The Mode is: %d \n", mode);
      // printf("Found it lba: %d %s \n", lba, a.dump().c_str());
      libRR_current_lba = lba;
      libRR_current_buffer = (void*)buf;
      libRR_run_script(a["code"]);
      return;
    }
  }
}