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

#ifndef COMMAND_HH
#define COMMAND_HH

#include <functional>
#include <string>
#include <vector>

namespace ben {
    using command_func = std::function<int(std::vector<std::string> const &)>;
    using help_func = std::function<void(std::string)>;

    void default_help(std::string cmd);

    void command_register(std::string const &cmd, command_func function,
                          help_func help = &default_help);
    int command_execute(std::vector<std::string> args);
    int show_help(std::string cmd);

    /* file.cc */
    void file_init();
    /* uni.cc */
    void uni_init();
    /* printer.cc */
    void printer_init();
    /* zlib.cc */
    void zlib_init();
} // namespace ben

#endif
