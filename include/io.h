#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

namespace io {

    namespace fs = std::filesystem;
    using byte_string = std::vector<char>;

    inline byte_string load(const fs::path& path) {
       if(!fs::exists(path))  throw std::runtime_error{ path.string() + " does not exists" };

       std::ifstream fin{path.string(), std::ios::binary | std::ios::ate};
       auto size = fin.tellg();
       fin.seekg(0);
       byte_string data(size);
       fin.read(data.data(), size);
       fin.close();

       return data;
    }
}


