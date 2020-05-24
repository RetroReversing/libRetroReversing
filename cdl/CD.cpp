#include "../include/libRR.h"
#include "CDL.hpp"
#include <fstream>
#include "../cd/kaitaistream.h"
#include "../cd/iso9660.h"
#include <sstream>

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
// 
void parse_sega_saturn(char* data, unsigned int data_length ) {
  start_pointer = &data[0];
  printf("parse_sega_saturn\n");
  seek(data, 0x9310, "Ignore Boot rom data");
  char value = read1Byte(data);
  string magic = readString(data, 5, "CD001");
  if (magic != "CD001") {
    return; // probably a audio track
  }
  json pvd = parse_primary_volume_descriptor(data);
  
  pvd = parse_path_table(data, pvd, pvd["path_table_size"]);

  // Now lets get top level files
  uint32_t lba = pvd["root_directory"]["extent_location_lba"];
  uint32_t directory_size = pvd["root_directory"]["extent_length"];
  pvd["root_files"] = parse_root_files(data, pvd, lba, directory_size);


  printf("pvd %s\n",pvd.dump(4).c_str());

}

std::vector<libRR_cd_track> libRR_cd_tracks;
void libRR_add_cd_track(string name, void* data, unsigned int data_length) {
  printf("\n\nlibRR_add_cd_track %s \n\n",name.c_str());
  libRR_cd_track track = {};
  track.data = data;
  track.length = data_length;
  track.name = name;
  libRR_cd_tracks.push_back(track);
  parse_sega_saturn((char*)data,data_length );
  // printf("Just about to load \n");
  // // const char buf[] = { ... };
  // // std::string str((char*)data, data_length);
  // // std::istringstream is(str);
  // std::ifstream is("/Users/alasdairmorrison/Downloads/RR/Saturn/Games/Devil Summoner - Soul Hackers (Japan) (Disc 1)/Devil Summoner - Soul Hackers (Japan) (Disc 1) (Track 1).bin", std::ifstream::binary);
  // printf("Just about to load ksteam\n");
  
  // kaitai::kstream ks(&is);
  // printf("Just about to load iso\n");
  // iso9660_t iso_data(&ks);
  // printf("Just about to load primary_vol_desc\n");
  // // auto a = iso_data.primary_vol_desc();
  // printf("after load vol desc\n");
  // try {
  // json b = iso_data.primary_vol_desc();
  // printf("Magic: %s \n", b.dump(4).c_str());
  // } catch( std::exception &e )
	// {
  //   printf("\nException: %s \n", e.what());
  // }
  // printf("Magic: %s \n", a->magic().c_str());
}