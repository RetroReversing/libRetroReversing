#include "CDL_FileWriting.hpp"

namespace codeDataLogger {
    std::string last_loaded_cdl;
    void copyOverOtherStatesDiffContents(std::ostream* file) {
        std::ifstream sourcecdlDiff(last_loaded_cdl.c_str(), std::ios_base::binary);
        for( std::string line; getline( sourcecdlDiff, line ); )
        {
            printf("Line: %s",line.c_str());
            *file << line << "\n";
        }
        sourcecdlDiff.close();
    }
    
    void writeInteger(std::ofstream* file, int num) {
        file->write(reinterpret_cast<const char *>(&num), sizeof(num));
    }
    
    void writeBlock(std::ofstream* file, const char *cdl_block, int size) {
        writeInteger(file,size);
        file->write(cdl_block, size);
    }
    
    void writeDataBytes(std::ostream* file, int start_range_address, int range_size, unsigned char* memory) {
        //*file << std::hex;
        for (int address =start_range_address; address < start_range_address+range_size; address++) {
            //*file << "0x";
            *file << (unsigned int)memory[address];
            if (address+1 < start_range_address+range_size)
                *file << ",";
        }
        //*file << std::dec;
    }
    
//
// # Reading Data
//    
    
    uint8_t readInteger8(std::ifstream* file) {
        uint8_t a;
        file->read (reinterpret_cast<char *>(&a), sizeof(a));
        return a;
    }
    
    uint32_t readInteger32(std::ifstream* file) {
        uint32_t a;
        file->read (reinterpret_cast<char *>(&a), sizeof(a));
        return a;
    }
    
    std::string readString(std::ifstream* file, int num) {
        std::string s(num /*bytes*/, '\0' /*initial content - irrelevant*/);
        file->read(&s[0], num /*bytes*/);
        return s;
    }
    
    void readBlock(std::ifstream* file, char* buffer) {
        
        uint8_t strSize = readInteger8(file);
        printf("string size: %d %d",(int) strSize, (int)file->tellg());
        std::string s = readString(file, strSize);
        printf("Block Name: %s",s.c_str());
        
        int size = readInteger32(file);
        printf("Block Size: %d",(int)size);
        file->read(buffer, size);
    }

    std::string readFileToString(std::string file_path) {
        std::ifstream t(file_path);
        std::string str;
        if (!t.good()) { return "File not found"; }

        t.seekg(0, std::ios::end);   
        str.reserve(t.tellg());
        t.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
        return str;
    }

    void writeStringToFile(std::string file_path, std::string contents) {
        std::ofstream out(file_path);
        if (!out.good()) { std::cout << "File not found" << file_path << "\n"; }
        out << contents;
        out.close();
    }

    void appendStringToFile(std::string file_path, std::string contents) {
        std::ofstream out;
        out.open(file_path, std::ios_base::app); // append instead of overwrite
        if (!out.good()) { std::cout << "File not found" << file_path << "\n"; }
        out << contents;
        out.close();
    }

    std::string dirnameOf(const std::string& fname)
    {
        size_t pos = fname.find_last_of("\\/");
        return (std::string::npos == pos)
            ? ""
            : fname.substr(0, pos);
    }
}
