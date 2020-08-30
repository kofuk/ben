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

#ifndef OPTION_HH
#define OPTION_HH

#include "file.hh"
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace ben {
    class option_matcher {
        std::vector<std::string> args;
        std::size_t cursor = 1;

    public:
        option_matcher(std::vector<std::string> args) : args(std::move(args)) {}

        std::string get_string();
        std::string get_string(std::string def);
        std::size_t select_string(std::vector<std::string>);
        std::size_t select_string(std::vector<std::string> item,
                                  std::size_t def_ind);
        std::size_t get_size();
        std::size_t get_size(std::size_t def);
        std::ptrdiff_t get_diff();
        std::ptrdiff_t get_diff(std::ptrdiff_t def);
        file *get_file_or_default();

        std::vector<std::string> get_rest();

        void must_not_remain();
    };
} // namespace ben

#endif
