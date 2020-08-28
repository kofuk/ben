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

#include <iostream>

#include <getopt.h>

#include "command.hh"
#include "file.hh"
#include "interactive.hh"

/* Simply, to make settings for code completion easy. */
#ifndef VERSION_MAJOR
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#endif

namespace {
    void print_usage() {
        std::cout << R"(usage: ben [OPTION]... [FILE]...
Load FILE's to buffer for analysis then launch interactive
command line.

  -h, --help     Print this message and exit.
  -v, --version  Print version and exit.
)";
    }

    void print_version() {
        std::cout << "ben "
                  << VERSION_MAJOR << '.' << VERSION_MINOR << '.'
                  << VERSION_PATCH << '\n';
    }

    struct option options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'},
        {0, 0, 0, 0},
    };

} // namespace

int main(int argc, char **argv) {
    for (;;) {
        int c = getopt_long(argc, argv, "hv", options, nullptr);
        if (c == -1) break;

        switch (c) {
        case 'h':
            print_usage();
            return 0;
        case 'v':
            print_version();
            return 0;
        default:
            return 1;
        }
    }

    ben::uni_init();
    ben::file_init();
    ben::printer_init();

    std::cout << "Loading files...\n";
    for (int i = optind; i < argc; ++i) {
        std::cout << " - Loading " << argv[i] << "...\n";
        ben::load_file(argv[i]);
    }
    ben::list_file();

    return ben::start_repl();
}
