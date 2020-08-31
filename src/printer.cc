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
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>

#include "command.hh"
#include "file.hh"
#include "option.hh"

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
            std::size_t en;
            try {
                option_matcher opt(args);
                en = opt.select_string({"little", "big"}, big_endian ? 1 : 0);
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "endian: " << e.what() << '\n';
                return 0;
            }

            if (en == 0) {
                big_endian = false;
            } else if (en == 1) {
                big_endian = true;
            } else {
                std::cout << (big_endian ? "big endian\n" : "little endian\n");
                return 0;
            }
            return 0;
        }

        void print_char(unsigned char const c) {
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
            enum print_type {
                CHAR,
                UINT8,
                UINT16,
                UINT32,
                UINT64,
                INT8,
                INT16,
                INT32,
                INT64,
                FLOAT,
                DOUBLE
            };
            print_type type;
            print_style style;
            file *f;
            try {
                option_matcher opt(args);
                type = static_cast<print_type>(opt.select_string(
                    {"char", "uint8", "uint16", "uint32", "uint64", "int8",
                     "int16", "int32", "int64", "float", "double"}));
                style = static_cast<print_style>(opt.select_string(
                    {"bin", "oct", "dec", "hex"},
                    static_cast<std::size_t>(print_style::DEC)));
                f = opt.get_file_or_default();
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "print: " << e.what() << '\n';
                return 1;
            }
            if (args.size() < 2) {
                help_print(args[0]);
                return 1;
            }

            std::ios init(nullptr);
            init.copyfmt(std::cout);

            switch (type) {
            case CHAR:
                if (!check_buffer_size(f, 1)) return 1;
                print_char(f->data[f->cursor]);
                std::cout << '\n';
                break;
            case UINT8:
                if (!check_buffer_size(f, 1)) return 1;
                print_value(f->data[f->cursor], style);
                break;
            case UINT16: {
                if (!check_buffer_size(f, 2)) return 1;
                uint16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                print_value(num, style);
                break;
            }
            case UINT32: {
                if (!check_buffer_size(f, 4)) return 1;
                uint32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
                break;
            }
            case UINT64: {
                if (!check_buffer_size(f, 8)) return 1;
                uint64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
                break;
            }
            case INT8:
                if (!check_buffer_size(f, 1)) return 1;
                print_value(f->data[f->cursor], style);
                break;
            case INT16: {
                if (!check_buffer_size(f, 2)) return 1;
                int16_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 2);
                print_value(num, style);
                break;
            }
            case INT32: {
                if (!check_buffer_size(f, 4)) return 1;
                int32_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
                break;
            }
            case INT64: {
                if (!check_buffer_size(f, 8)) return 1;
                int64_t num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
                break;
            }
            case FLOAT: {
                if (!check_buffer_size(f, 4)) return 1;
                float num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 4);
                print_value(num, style);
                break;
            }
            case DOUBLE: {
                if (!check_buffer_size(f, 8)) return 1;
                double num;
                ordered_memcpy(&num, f->data.data() + f->cursor, 8);
                print_value(num, style);
                break;
            }
            default:
                std::cout << "Unknown type.\n";
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
            std::size_t len;
            file *f;
            try {
                option_matcher opt(args);
                len = opt.get_size(0);
                f = opt.get_file_or_default();
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "string: " << e.what() << '\n';
                return 1;
            }

            if (len != 0) {
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

        void help_xd([[maybe_unused]] std::string cmd) {
            std::cout << R"(usage: xd [BUF]
Dump bytes around cursor like xxd(1).
)";
        }

        int xd(std::vector<std::string> const &args) {
            file *f;
            try {
                option_matcher opt(args);
                f = opt.get_file_or_default();
                opt.must_not_remain();
            } catch (std::runtime_error const &e) {
                std::cout << "xd: " << e.what() << '\n';
                return 1;
            }

            std::ios init(nullptr);
            init.copyfmt(std::cout);

            std::size_t beg = f->cursor & ~0xful;
            std::size_t en = beg + 0x100;
            for (std::size_t pos = beg;; ++pos) {
                if ((pos & 0xf) == 0 && pos != beg) {
                    std::cout << ' ';
                    for (std::size_t alpos = (pos - 1) & ~0xful; alpos < pos;
                         ++alpos) {
                        if (alpos == f->cursor) std::cout << "\e[1;7m";
                        if (std::isprint(f->data[alpos])) {
                            std::cout << f->data[alpos];
                        } else {
                            std::cout << '.';
                        }
                        if (alpos == f->cursor) std::cout << "\e[0m";
                    }
                    std::cout << '\n';
                    if (pos == f->data.size() || pos == en) break;
                }

                if (pos >= f->data.size()) {
                    std::cout.copyfmt(init);
                    if (pos & 0b1) {
                        std::cout << "  ";
                        ++pos;
                    }
                    for (unsigned int i = 0; i < (8 - (pos & 0xf) / 2); ++i) {
                        std::cout << "     ";
                    }
                    std::cout << "  ";
                    for (std::size_t alpos = pos & ~0xful;
                         alpos < f->data.size(); ++alpos) {
                        if (alpos == f->cursor) std::cout << "\e[1;7m";
                        if (std::isprint(f->data[alpos])) {
                            std::cout << f->data[alpos];
                        } else {
                            std::cout << '.';
                        }
                        if (alpos == f->cursor) std::cout << "\e[0m";
                    }
                    std::cout << '\n';
                    break;
                }

                if ((pos & 0xf) == 0) {
                    std::cout << std::setw(8) << std::setfill('0') << std::hex
                              << pos << ": ";
                }

                if (pos == f->cursor) {
                    std::cout << "\e[1;7m";
                }
                std::cout << std::setw(2) << std::setfill('0') << std::hex
                          << +f->data[pos];
                if (pos == f->cursor) {
                    std::cout << "\e[0m";
                }

                if (pos & 0b1) {
                    std::cout << ' ';
                }
            }

            std::cout.copyfmt(init);

            return 0;
        }
    } // namespace

    void printer_init() {
        command_register("print", &print, &help_print);
        command_register("endian", &endian, &help_endian);
        command_register("string", &str, &help_str);
        command_register("xd", &xd, &help_xd);
    }
} // namespace ben
