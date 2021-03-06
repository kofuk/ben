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
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <readline/history.h>
#include <readline/readline.h>

#include "command.hh"
#include "interactive.hh"
#include "parse.hh"
#include "variable.hh"

namespace ben {
    namespace {
        void execute_command_line(char const *cmd) {
            command_chain *chain = parse_command_line(cmd);
            command_chain *first = chain;
            while (chain != nullptr) {
                chain->stmt->execute();

                chain = chain->next;
            }
            command_chain_clean_up(first);
        }

        bool repl_exited = false;
    } // namespace

    int start_repl() {
        std::signal(SIGINT, SIG_IGN);

        ::using_history();
        for (;;) {
            char *line = ::readline(lookup_variable("PROMPT").c_str());
            if (!line) {
                break;
            }

            try {
                execute_command_line(lookup_variable("PRE_COMMAND").c_str());
                execute_command_line(line);
                if (repl_exited) {
                    std::free(line);
                    break;
                }

                execute_command_line(lookup_variable("POST_COMMAND").c_str());
            } catch (std::exception const &e) {
                std::cout << e.what() << '\n';
            }
            if (std::strlen(line)) {
                ::add_history(line);
            }

            std::free(line);
        }

        std::cout << "exit\n";
        return 0;
    }

    void exit_repl() {
        repl_exited = true;
    }
} // namespace ben
