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

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include <readline/readline.h>

#include "command.hh"
#include "interactive.hh"

namespace ben {
    namespace {
        std::vector<std::string> split_cmdline(std::string_view const &line) {
            std::vector<std::string> result;

            std::string cur;
            bool quoted = false;
            bool escaped = false;
            for (char c : line) {
                if (c == '"' && !escaped) {
                    quoted = !quoted;
                    continue;
                } else if (c == '\\') {
                    escaped = true;
                    continue;
                }
                if (c == ' ') {
                    if (!quoted && !cur.empty()) {
                        result.push_back(cur);
                        cur.clear();
                        continue;
                    }
                } else if (escaped) {
                    switch (c) {
                    case 'a':
                        c = '\a';
                        break;
                    case 'e':
                        c = '\e';
                        break;
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    }
                }
                cur.push_back(c);

                escaped = false;
            }
            if (!cur.empty()) result.push_back(cur);

            return result;
        }
    } // namespace

    int start_repl() {
        std::signal(SIGINT, SIG_IGN);

        char *prev = nullptr;
        for (;;) {
            char *line = ::readline("ben> ");
            if (!line) {
                std::free(prev);
                break;
            }

            if (!std::strlen(line) && prev) {
                std::free(line);
                line = prev;
            } else {
                std::free(prev);
            }
            std::vector<std::string> args =
                split_cmdline(std::string_view(line));
            if (!args.empty()) {
                if (args[0] == "help") {
                    if (args.size() < 2) {
                        std::cout << "usage: help COMMAND\n";
                    } else {
                        show_help(args[1]);
                    }
                } else {
                    command_execute(args);
                }
            }

            prev = line;
        }

        std::cout << "exit\n";
        return 0;
    }
} // namespace ben
