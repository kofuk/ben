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

#include <unordered_map>

#include "variable.hh"

namespace ben {
    namespace {
        std::unordered_map<std::string, std::string> variable_map;
    }

    std::string lookup_variable(std::string const &name) {
        auto result = variable_map.find(name);
        if (result == variable_map.end()) {
            return "";
        }
        return result->second;
    }

    void add_variable(std::string const &key, std::string const &value) {
        variable_map[key] = value;
    }
} // namespace ben
