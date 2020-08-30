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

#ifndef VARIABLE_HH
#define VARIABLE_HH

#include <string>

namespace ben {
    std::string lookup_variable(std::string const &name);
    void add_variable(std::string const &key, std::string const &value);

    void set_initial_variables();

    bool is_truthy(std::string const &expr);
    bool is_falsy(std::string const &expr);
} // namespace ben

#endif
