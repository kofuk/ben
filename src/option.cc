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
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "file.hh"
#include "option.hh"

namespace ben {
    std::string option_matcher::get_string() {
        using namespace std::string_literals;
        if (cursor < args.size()) {
            return args[cursor++];
        }
        throw std::runtime_error("Mandatory argument omitted."s);
    }

    std::string option_matcher::get_string(std::string def) {
        if (cursor < args.size()) {
            return args[cursor++];
        }
        return def;
    }

    std::size_t option_matcher::select_string(std::vector<std::string> item) {
        using namespace std::string_literals;
        if (cursor < args.size()) {
            std::string &it = args[cursor++];

            for (std::size_t i = 0; i < item.size(); ++i) {
                if (item[i] == it) {
                    return i;
                }
            }

            throw std::runtime_error("Arg value is not allowed."s);
        }
        throw std::runtime_error("Mandatory argument omitted."s);
    }

    std::size_t option_matcher::select_string(std::vector<std::string> item,
                                              std::size_t def_ind) {
        if (cursor < args.size()) {
            std::string &it = args[cursor++];

            for (std::size_t i = 0; i < item.size(); ++i) {
                if (item[i] == it) {
                    return i;
                }
            }

            throw std::runtime_error("Arg value is not allowed.");
        }
        return def_ind;
    }

    std::size_t option_matcher::get_size() {
        using namespace std::string_literals;
        try {
            if (cursor < args.size()) {
                std::size_t val = std::stoul(args[cursor++], nullptr, 0);
                return val;
            }
        } catch (std::invalid_argument const &) {
            throw std::runtime_error("Expect integer value."s);
        } catch (std::range_error const &) {
            throw std::runtime_error("Argument is out of range"s);
        }
        throw std::runtime_error("Mandatory argument omitted."s);
    }

    std::size_t option_matcher::get_size(std::size_t def) {
        using namespace std::string_literals;
        try {
            if (cursor < args.size()) {
                std::size_t val = std::stoul(args[cursor++], nullptr, 0);
                return val;
            }
        } catch (std::invalid_argument const &) {
            throw std::runtime_error("Expect integer value."s);
        } catch (std::range_error const &) {
            throw std::runtime_error("Argument is out of range"s);
        }
        return def;
    }

    std::ptrdiff_t option_matcher::get_diff() {
        using namespace std::string_literals;
        try {
            if (cursor < args.size()) {
                std::ptrdiff_t val = std::stol(args[cursor++], nullptr, 0);
                return val;
            }
        } catch (std::invalid_argument const &) {
            throw std::runtime_error("Expect integer value."s);
        } catch (std::range_error const &) {
            throw std::runtime_error("Argument is out of range"s);
        }
        throw std::runtime_error("Mandatory argument omitted."s);
    }

    std::ptrdiff_t option_matcher::get_diff(std::ptrdiff_t def) {
        using namespace std::string_literals;
        try {
            if (cursor < args.size()) {
                std::ptrdiff_t val = std::stol(args[cursor++], nullptr, 0);
                return val;
            }
        } catch (std::invalid_argument const &) {
            throw std::runtime_error("Expect integer value."s);
        } catch (std::range_error const &) {
            throw std::runtime_error("Argument is out of range"s);
        }
        return def;
    }

    file *option_matcher::get_file_or_default() {
        using namespace std::string_literals;
        if (cursor < args.size()) {
            std::string arg = args[cursor++];
            if (arg.size() < 2 || arg[0] != '%' ||
                arg.find_first_not_of("0123456789", 1) != std::string::npos) {
                throw std::runtime_error("Invalid buffer representation.");
            }
            file *f =  get_file(arg);
            if (!f) {
                throw std::runtime_error("Buffer not found."s);
            }
        }
        file *f = get_file(""s);
        if (!f) {
            throw std::runtime_error("No default buffer selected."s);
        }
        return f;
    }

    std::vector<std::string> option_matcher::get_rest() {
        std::vector<std::string> result;
        result.insert(result.end(), args.begin() + cursor, args.end());
        cursor = args.size() - 1;
        return result;
    }

    void option_matcher::must_not_remain() {
        if (cursor != args.size()) {
            throw std::runtime_error("Too many arguments");
        }
    }
} // namespace ben
