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
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <zlib.h>

#include "command.hh"
#include "file.hh"
#include "option.hh"

namespace ben {
    namespace {
        void help_zlib([[maybe_unused]] std::string cmd) {
            std::cout << R"(usage: zlib LEN [BUF]
Deflate specified region of the buffer and add decompressed
byte array as a new buffer.
)";
        }

        std::vector<std::uint8_t>
        zlib_inflate(std::unique_ptr<unsigned char[]> buf, std::size_t len) {
            int z_ret;
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = 0;
            strm.next_in = Z_NULL;

            if (inflateInit(&strm) != Z_OK) {
                throw std::runtime_error("Failed to initialize zlib.");
            }
            std::vector<std::uint8_t> result;
            unsigned char out[1024];

            strm.avail_in = len;
            strm.next_in = buf.get();

            do {
                strm.avail_out = 1024;
                strm.next_out = out;

                z_ret = inflate(&strm, Z_NO_FLUSH);

                if (z_ret == Z_STREAM_ERROR) {
                    inflateEnd(&strm);
                    throw std::runtime_error("zlib error: stream error");
                } else if (z_ret == Z_NEED_DICT) {
                    inflateEnd(&strm);
                    throw std::runtime_error("zlib error: need dictionary");
                } else if (z_ret == Z_DATA_ERROR) {
                    inflateEnd(&strm);
                    throw std::runtime_error("zlib error: data error");
                } else if (z_ret == Z_MEM_ERROR) {
                    inflateEnd(&strm);
                    throw std::runtime_error("zlib error: memory error");
                }

                result.insert(std::end(result), buf.get(),
                              buf.get() + 1024 - strm.avail_out);
            } while (strm.avail_out != 0);

            if (z_ret != Z_STREAM_END) {
                inflateEnd(&strm);
                throw std::runtime_error(
                    "zlib error: decompressed buffer is not complete.");
            }

            inflateEnd(&strm);
            return result;
        }

        int zlib(std::vector<std::string> const &args) {
            std::size_t len;
            file *f;
            try {
                option_matcher opt(args);
                len = opt.get_size();
                f = opt.get_file_or_default();
                opt.must_not_remain();
            } catch (std::exception const &e) {
                std::cout << "zlib: " << e.what() << '\n';
                return 1;
            }

            if (f->data.size() - f->cursor < len) {
                std::cout << "zlib: LEN exceeds buffer.\n";
                return 1;
            }

            try {
                std::unique_ptr<unsigned char[]> buf =
                    std::make_unique<unsigned char[]>(len);
                std::copy(f->data.cbegin() + f->cursor,
                          f->data.cbegin() + f->cursor + len, buf.get());
                std::vector<std::uint8_t> result =
                    zlib_inflate(std::move(buf), len);
                int han = add_file_buffer(
                    f->filename + "#z" + std::to_string(f->cursor), result);
                std::cout << "Added as %" << han << '\n';
            } catch (std::exception const &e) {
                std::cout << e.what() << '\n';
                return 1;
            }

            return 0;
        }
    } // namespace

    void zlib_init() { command_register("zlib", &zlib, &help_zlib); }
} // namespace ben
