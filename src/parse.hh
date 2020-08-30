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

#ifndef PARSE_HH
#define PARSE_HH

#include <string>
#include <vector>

namespace ben {
    enum class command_type { COMMAND, ASSIGN };

    class single_statement {
    public:
        virtual ~single_statement() {}
        virtual int execute() = 0;
    };

    class assignment_statement : public single_statement {
        std::string lhs;
        std::string rhs;

    public:
        assignment_statement(std::string lhs, std::string rhs)
            : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

        int execute() override;
    };

    class command_statement : public single_statement {
        std::vector<std::string> command_line;

    public:
        void push_arg(std::string arg) { command_line.push_back(arg); }

        int execute() override;
    };

    struct command_chain {
        command_type type;
        single_statement *stmt;
        command_chain *next = nullptr;
    };

    command_chain *parse_command_line(std::string commandline);

    void command_chain_clean_up(command_chain *obj);
} // namespace ben

#endif
