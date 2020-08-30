/*
 * Interactive binary viewer.
 * Copyright (C) 2020  Koki Fukuda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "command.hh"
#include "file.hh"
#include "option.hh"

namespace ben {
    namespace {
        void help_seek([[maybe_unused]] std::string cmd) {
            std::cout << R"(usage: seek COUNT [BUF] [BASE]
If BASE is omitted, seeks N bytes relative to current cursor.
Both positive and negative COUNT is allowed.
Negative BASE means BASE bytes from the end of the buffer.
)";
        }

        int seek(std::vector<std::string> const &args) {
            std::ptrdiff_t count;
            file *f;
            std::ptrdiff_t n;
            try {
                option_matcher opt(args);
                count = opt.get_diff();
                f = opt.get_file_or_default();
                n = opt.get_diff(f->cursor);
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "seek: " << e.what() << '\n';
                return 1;
            }

            std::size_t base;
            if (n >= 0) {
                if (static_cast<std::size_t>(n) < f->data.size()) {
                    base = n;
                } else {
                    std::cout << "BASE exceeds buffer.\n";
                    return 1;
                }
            } else {
                if (static_cast<std::size_t>(-n) < f->data.size()) {
                    base = f->data.size() + n;
                } else {
                    std::cout << "BASE exceeds buffer.\n";
                    return 1;
                }
            }

            if (count >= 0) {
                if (base + count < f->data.size()) {
                    f->cursor = base + count;
                } else {
                    std::cout << "Cursor exceeds buffer.\n";
                    return 1;
                }
            } else {
                if (static_cast<std::size_t>(-count) < base) {
                    f->cursor = base + count;
                } else {
                    std::cout << "Cursor exceeds buffer.\n";
                    return 1;
                }
            }

            return 0;
        }

        void help_load([[maybe_unused]] std::string cmd) {
            std::cout << "usage: load FILE\n";
        }

        int load(std::vector<std::string> const &args) {
            std::string name;
            try {
                option_matcher opt(args);
                name = opt.get_string();
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "load: " << e.what() << '\n';
                return 1;
            }

            load_file(name);
            list_file();

            return 0;
        }

        int ls_buf([[maybe_unused]] std::vector<std::string> const &args) {
            if (args.size() >= 2) {
                std::cout << "lsbuf: Too many arguments.\n";
                return 1;
            }
            list_file();
            return 0;
        }

        unsigned int default_file_num;

        std::vector<std::uint8_t> load_file_stdin() {
            std::vector<std::uint8_t> data;
            char buf[2048];

            do {
                std::cin.read(buf, 2048);
                data.insert(data.end(), buf, buf + std::cin.gcount());
            } while (!std::cin.fail());
            std::cin.clear();

            return data;
        }
    } // namespace

    std::vector<file> files;

    void file_init() {
        command_register("seek", &seek, &help_seek);
        command_register("load", &load, &help_load);
        command_register("lsbuf", &ls_buf);
    }

    int load_file(std::string filename) {
        std::vector<std::uint8_t> data;

        if (filename == "-") {
            data = load_file_stdin();
        } else {

            std::ifstream strm(filename, std::ios::binary);
            if (!strm) {
                int errsave = errno;
                std::cout << "Failed to load: " << std::strerror(errsave) << '\n';
                return -1;
            }

            char buf[2048];
            do {
                strm.read(buf, 2048);
                data.insert(data.end(), buf, buf + strm.gcount());
            } while (!strm.fail());

            if (!strm.eof()) {
                std::cout << "Error loading " << filename
                          << "; file may not be complete.\n";
            }
        }

        file f;
        f.filename = filename == "-" ? "*stdin*" : filename;
        f.data = data;
        files.push_back(f);

        return files.size() - 1;
    }

    int add_file_buffer(std::string filename, std::vector<std::uint8_t> &buf) {
        file f;
        f.filename = filename;
        f.data = buf;
        files.push_back(f);

        return files.size() - 1;
    }

    file *get_file(std::string repr) {
        if (repr.empty()) {
            if (default_file_num < files.size()) {
                return &files[default_file_num];
            } else {
                return nullptr;
            }
        }

        if (repr.size() < 2) return nullptr;

        unsigned int n;
        try {
            n = std::stoi(repr.substr(1));
        } catch (std::exception const &) {
            return nullptr;
        }
        if (n >= files.size()) {
            return nullptr;
        }
        default_file_num = n;
        return &files[n];
    }

    void list_file() {
        for (unsigned int i = 0; i < files.size(); ++i) {
            std::cout << " %" << i << ": " << files[i].filename << '\n';
        }
    }
} // namespace ben
