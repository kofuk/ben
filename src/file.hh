#ifndef FILE_HH
#define FILE_HH

#include <string>
#include <vector>

namespace ben {
    struct file {
        std::string filename;
        std::vector<std::uint8_t> data;
        std::size_t cursor = 0;
    };

    int load_file(std::string filename);
    file *get_file(std::string repr);
    void list_file();
}

#endif
