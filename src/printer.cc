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
#include <bitset>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>

#include "command.hh"
#include "file.hh"

namespace ben {
    namespace {
        bool big_endian;

        void help_endian([[maybe_unused]] std::string cmd) {
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

        void print_char(char const c) {
            if (std::isprint(c)) {
                std::cout << c;
            } else {
                std::ios init(nullptr);
                init.copyfmt(std::cout);
                std::cout << "\\x" << std::hex << std::setw(2)
                          << std::setfill('0') << +c;
                std::cout.copyfmt(init);
            }
        }

        void help_print([[maybe_unused]] std::string cmd) {
            std::cout << R"(print TYPE [bin|oct|dec|hex] [BUFFER]
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

        enum class print_style { BIN, OCT, DEC, HEX };

        template <typename T> void print_value(T value, print_style sty) {
            if (sty == print_style::BIN) {
                std::cout << std::bitset<sizeof(T) * 8>(value) << '\n';
            } else {
                std::ios init(nullptr);
                init.copyfmt(std::cout);

                if (sty == print_style::OCT) {
                    std::cout << std::oct;
                } else if (sty == print_style::DEC)
                    std::cout << std::dec;
                else if (sty == print_style::HEX)
                    std::cout << std::hex;

                std::cout << +value << '\n';

                std::cout.copyfmt(init);
            }
        }

        int print(std::vector<std::string> const &args) {
            using namespace std::string_literals;
            if (args.size() < 2) {
                help_print(args[0]);
                return 1;
            }

            file *f = NULL;
            print_style style = print_style::DEC;

            if (args.size() >= 3) {
                if (args[2] == "bin") {
                    style = print_style::BIN;
                } else if (args[2] == "oct") {
                    style = print_style::OCT;
                } else if (args[2] == "dec") {
                    style = print_style::DEC;
                } else if (args[2] == "hex") {
                    style = print_style::HEX;
                } else {
                    f = get_file(args[2]);
                }
                if (!f) {
                    if (args.size() == 3) {
                        f = get_file(""s);
                    } else {
                        f = get_file(args[3]);
                    }
                }
            } else {
                f = get_file(""s);
            }
            if (!f) {
                std::cout << "Invalid buffer.\n";
                return 1;
            }

            std::ios init(nullptr);
            init.copyfmt(std::cout);

            std::string type = args[1];
            if (type == "char"s) {
                if (!check_buffer_size(f, 1)) return 1;
                print_char(f->data[f->cursor]);
                std::cout << '\n';
            } else if (type == "uint8"s) {
                if (!check_buffer_size(f, 1)) return 1;
                print_value(f->data[f->cursor], style);
            } else if (type == "uint16"s) {
                if (!check_buffer_size(f, 2)) return 1;
                uint16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                print_value(num, style);
            } else if (type == "uint32"s) {
                if (!check_buffer_size(f, 4)) return 1;
                uint32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
            } else if (type == "uint64"s) {
                if (!check_buffer_size(f, 8)) return 1;
                uint64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
            } else if (type == "int8"s) {
                if (!check_buffer_size(f, 1)) return 1;
                print_value(f->data[f->cursor], style);
            } else if (type == "int16"s) {
                if (!check_buffer_size(f, 2)) return 1;
                int16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                print_value(num, style);
            } else if (type == "int32"s) {
                if (!check_buffer_size(f, 4)) return 1;
                int32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
            } else if (type == "int64"s) {
                if (!check_buffer_size(f, 8)) return 1;
                int64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
            } else if (type == "float"s) {
                if (!check_buffer_size(f, 4)) return 1;
                float num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
            } else if (type == "double"s) {
                if (!check_buffer_size(f, 8)) return 1;
                double num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
            } else {
                std::cout << "Unknown type\n";
                return 1;
            }

            return 0;
        }

        void help_str([[maybe_unused]] std::string cmd) {
            std::cout << R"(usage: string [LEN [BUF]]
Print LEN bytes beginning from cursor as ASCII string.
If LEN is not specified, prints following printable ASCII
characters.
)";
        }

        int str(std::vector<std::string> const &args) {
            bool len_specified = false;
            std::size_t len = 0;
            file *f;
            if (args.size() >= 2) {
                len_specified = true;
                try {
                    len = std::stoul(args[1]);
                } catch (std::exception const &) {
                    std::cout << "Invalid length.\n";
                    return 1;
                }
                if (args.size() >= 3) {
                    f = get_file(args[2]);
                } else {
                    f = get_file("");
                }
            } else {
                f = get_file("");
            }

            if (!f) {
                std::cout << "Invalid buffer.\n";
                return 1;
            }

            if (len_specified) {
                for (std::size_t point = f->cursor;
                     point < f->data.size() && point < f->cursor + len; ++point)
                    print_char(f->data[point]);

                std::cout << '\n';
            } else {
                bool any = false;
                for (std::size_t point = f->cursor; point < f->data.size();
                     ++point) {
                    if (std::isprint(f->data[point])) {
                        print_char(f->data[point]);
                        any = true;
                    } else {
                        break;
                    }
                }

                if (any) std::cout << '\n';
            }

            return 0;
        }
    } // namespace

    void printer_init() {
        command_register("print", &print, &help_print);
        command_register("endian", &endian, &help_endian);
        command_register("string", &str, &help_str);
    }
} // namespace ben
