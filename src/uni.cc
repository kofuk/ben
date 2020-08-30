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

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef __unix__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "command.hh"
#include "interactive.hh"

namespace ben {
    namespace {
        int echo(std::vector<std::string> const &args) {
            for (size_t i = 1; i < args.size(); ++i) {
                if (i != 1) {
                    std::cout << ' ';
                }
                std::cout << args[i];
            }
            std::cout << '\n';
            return 0;
        }

        int exit([[maybe_unused]]std::vector<std::string> const &args) {
            exit_repl();
            return 0;
        }

#ifdef __unix__
        void help_command([[maybe_unused]] std::string cmd) {
            std::cout << "usage: command COMMAND [ARG]...\n";
        }

        int command(std::vector<std::string> const &args) {
            if (args.size() < 2) return 0;

            ::pid_t pid = ::fork();
            if (pid == -1) {
                int errsave = errno;
                std::cout << "command:" << std::strerror(errsave) << '\n';
                return 1;
            }
            if (pid == 0) {
                char *argv[args.size()];
                for (unsigned int i = 1; i < args.size(); ++i) {
                    argv[i - 1] = const_cast<char *>(args[i].c_str());
                }
                argv[args.size() - 1] = 0;
                ::execvp(args[1].c_str(), argv);

                int errsave = errno;
                std::cout << "command: " << args[1] << ": "
                          << ::strerror(errsave) << '\n';
                std::exit(1);
            }
            int status;
            ::waitpid(pid, &status, 0);
            return status;
        }

        int cd(std::vector<std::string> const &args) {
            if (args.size() > 2) {
                std::cout << "cd: Too many arguments.\n";
            }
            char const *dst;
            if (args.size() < 2) {
                dst = ::getenv("HOME");
                if (dst == nullptr) {
                    std::cout << "cd: HOME is not set.";
                }
            } else {
                dst = args[1].c_str();
            }
            if (::chdir(dst)) {
                int errsave = errno;
                std::cout << "cd: " << dst << ": " << std::strerror(errsave)
                          << '\n';
            }

            return 0;
        }

        int pwd([[maybe_unused]] std::vector<std::string> const &args) {
            char *cwd = ::get_current_dir_name();
            if (!cwd) {
                int errsave = errno;
                std::cout << "pwd: " << std::strerror(errsave) << '\n';
                return 1;
            }
            std::cout << cwd << '\n';
            std::free(cwd);
            return 0;
        }
#endif
    } // namespace

    void uni_init() {
        using namespace std::string_literals;

        command_register("echo"s, &echo);
        command_register("exit", &exit);
#ifdef __unix__
        command_register("command"s, &command, &help_command);
        command_register("cd"s, &cd);
        command_register("pwd"s, &pwd);
#endif
    }
} // namespace ben
