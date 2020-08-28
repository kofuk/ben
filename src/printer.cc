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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "command.hh"
#include "file.hh"

namespace ben {
    namespace {
        void help_print([[maybe_unused]]std::string cmd) {
            std::cout << R"(print TYPE [BUFFER]
Interpret byte array beginning from cursor position as given TYPE
and print.
You can check and specify byte-order with `endian' command.
If BUFFER is omitted, use previously used buffer.

Possible types;
  char    ASCII character.
  uint8   unsigned int with 8-bit width.
  uint16  unsigned int with 16-bit width.
  uint32  unsigned int with 32=bit width.
  uint64  unsigned int with 64-bit width.
  int8    two's complement 8-bit integer.
  int16   two's complement 16-bit integer.
  int32   two's complement 32-bit integer.
  int64   two's complement 64-bit integer.
  float   IEEE 754 single precision floating point number.
  double  IEEE 754 double precision floating point number.
)";
        }

        bool big_endian;

        void help_endian([[maybe_unused]]std::string cmd) {
            std::cout << R"(usage: endian [big|little]
       endian

if big or little is specified, prints current value.
)";
        }

        int endian(std::vector<std::string> const &args) {
            if (args.size() < 2) {
                std::cout << (big_endian ? "big endian\n" : "little endian\n");
                return 0;
            }
            if (args[1] == "little") {
                big_endian = false;
            } else if (args[1] == "big") {
                big_endian = true;
            } else {
                std::cout << "Unknown byte order\n";
                return 1;
            }
            return 0;
        }

        inline void ordered_memcpy(void *dest, void *src, size_t n) {
            if (big_endian) {
                std::uint8_t buf[n];
                std::memcpy(buf, src, n);
                std::reverse(buf, buf + n);
                std::memcpy(dest, buf, n);
            } else {
                std::memcpy(dest, src, n);
            }
        }

        bool check_buffer_size(file *f, unsigned int size) {
            return f->data.size() - f->cursor >= size;
        }

        int print(std::vector<std::string> const &args) {
            using namespace std::string_literals;
            if (args.size() < 2) {
                help_print(args[0]);
                return 1;
            }
            file *f;
            if (args.size() >= 3) {
                f = get_file(args[1]);
            } else {
                f = get_file(""s);
            }
            if (!f) {
                std::cout << "Invalid buffer.\n";
                return 1;
            }

            std::string type = args[1];
            if (type == "char"s) {
                if (!check_buffer_size(f, 1)) return 1;
                std::cout << (char)f->data[f->cursor] << '\n';
            } else if (type == "uint8"s) {
                if (!check_buffer_size(f, 1)) return 1;
                std::cout << (unsigned int)f->data[f->cursor] << '\n';
            } else if (type == "uint16"s) {
                if (!check_buffer_size(f, 2)) return 1;
                uint16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                std::cout << num << '\n';
            } else if (type == "uint32"s) {
                if (!check_buffer_size(f, 4)) return 1;
                uint32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                std::cout << num << '\n';
            } else if (type == "uint64"s) {
                if (!check_buffer_size(f, 8)) return 1;
                uint64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                std::cout << num << '\n';
            } else if (type == "int8"s) {
                if (!check_buffer_size(f, 1)) return 1;
                std::cout << (int)f->data[f->cursor] << '\n';
            } else if (type == "int16"s) {
                if (!check_buffer_size(f, 2)) return 1;
                int16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                std::cout << num << '\n';
            } else if (type == "int32"s) {
                if (!check_buffer_size(f, 4)) return 1;
                int32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                std::cout << num << '\n';
            } else if (type == "int64"s) {
                if (!check_buffer_size(f, 8)) return 1;
                int64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                std::cout << num << '\n';
            } else if (type == "float"s) {
                if (!check_buffer_size(f, 4)) return 1;
                float num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                std::cout << num << '\n';
            } else if (type =="double"s) {
                if (!check_buffer_size(f, 8)) return 1;
                double num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                std::cout << num << '\n';
            } else {
                std::cout << "Unknown type\n";
                return 1;
            }

            return 0;
        }

    }

    void printer_init() {
        command_register("print", &print, &help_print);
        command_register("endian", &endian, &help_endian);
    }
}
