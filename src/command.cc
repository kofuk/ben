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

#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#include "command.hh"
#include "modes.hh"
#include "option.hh"
#include "variable.hh"

namespace ben {
    namespace {
        struct command {
            command_func cmd;
            help_func help;
        };

        std::unordered_map<std::string, command> command_map;

        int help(std::vector<std::string> const &args) {
            if (args.size() < 2) {
                for (auto itr = command_map.cbegin(), E = command_map.cend();
                     itr != E; ++itr) {
                    std::cout << itr->first << '\n';
                }
                return 0;
            }

            return show_help(args[1]);
        }

        void help_mode(std::string const) {
            std::cout << R"(usage: mode KEY VALUE
If VALUE is empty or not specified, show current value.
Possible keys;
  auto-shell    If on, try executing system command if ben command not found.
)";
        }

        int mode(std::vector<std::string> const &args) {
            std::string key;
            std::string value;
            try {
                option_matcher opt(args);
                key = opt.get_string();
                value = opt.get_string("");
                opt.must_not_remain();
            } catch (std::exception const &e) {
                std::cout << "mode: " << e.what() << '\n';
                return 1;
            }

            if (key == "auto-shell") {
                if (value.empty()) {
                    std::cout << (modes::auto_shell ? "ON" : "OFF") << '\n';
                } else {
                    modes::auto_shell = is_truthy(value);
                }
            }
            return 0;
        }
    } // namespace

    void command_register(std::string const &cmd, command_func function,
                          help_func help) {
        if (command_map.find(cmd) != std::end(command_map)) {
            std::cout << "Warning: " + cmd + " got redefined.";
        }
        command c;
        c.cmd = function;
        c.help = help;
        command_map[cmd] = c;
    }

    int command_execute(std::vector<std::string> args) {
        if (args.empty()) {
            return 255;
        }
    retry:
        auto itr = command_map.find(args[0]);
        if (itr == std::end(command_map)) {
            if (args[0] != "command" && modes::auto_shell) {
                args.insert(args.begin(), "command");
                goto retry;
            } else {
                std::cout << "ben: " << args[0] << ": command not found\n";
                return 255;
            }
        }

        int retval;
        try {
            retval = itr->second.cmd(args);
        } catch (std::exception const &e) {
            std::cout << "BUG: " << e.what() << '\n';
            return 255;
        }
        return retval;
    }

    int show_help(std::string cmd) {
        auto itr = command_map.find(cmd);
        if (itr == std::end(command_map)) {
            std::cout << "ben: " << cmd << ": command not found\n";
            return 255;
        }

        itr->second.help(cmd);
        return 0;
    }

    void default_help(std::string cmd) {
        std::cout << "Help for " << cmd << " is not provided.\n";
    }

    void command_init() {
        command_register("help", &help);
        command_register("mode", &mode, &help_mode);
    }
} // namespace ben
