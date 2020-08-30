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
            if (args[0] != "command" && is_truthy(lookup_variable("_AUTO_SHELL_"))) {
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

    void command_init() { command_register("help", &help); }
} // namespace ben
