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
#include <cctype>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "command.hh"
#include "parse.hh"

namespace ben {
    namespace {
        enum class token_type {
            NONE,
            STRING,
            END_STMT,
        };

        struct token {
            token_type type = token_type::NONE;
            std::size_t begin = 0;
            std::size_t end = 0;
        };

        std::size_t tokenize_quoted(std::string const &commandline,
                                    std::size_t pos) {
            char end;
            if (commandline[pos] == '\'') {
                end = '\'';
            } else {
                end = '"';
            }
            bool escaped = false;
            for (; pos < commandline.size(); ++pos) {
                if (escaped) {
                    escaped = false;
                    continue;
                }
                if (commandline[pos] == '\\') escaped = true;
                if (commandline[pos] == end) return pos;
            }

            throw std::runtime_error("parse error at " + std::to_string(pos));
        }

        std::vector<token> tokenize(std::string const &commandline) {
            token current;
            std::vector<token> tokens;
            bool escaped = false;
            for (size_t i = 0; i < commandline.size(); ++i) {

#define PUSH_CURRENT()                          \
    do {                                        \
        if (current.type != token_type::NONE) { \
            current.end = i;                    \
            tokens.push_back(current);          \
        }                                       \
    } while (0)

#define RENEW_TOKEN(ty)      \
    do {                     \
        current.type = (ty); \
        current.begin = i;   \
    } while (0)

                if (escaped) {
                    escaped = false;
                    continue;
                }

                switch (commandline[i]) {
                case ';':
                    PUSH_CURRENT();
                    RENEW_TOKEN(token_type::END_STMT);
                    break;

                case '"':
                    if (current.type != token_type::STRING) {
                        current.begin = i;
                        current.type = token_type::STRING;
                    }
                    i = tokenize_quoted(commandline, i);
                    break;

                case ' ':
                    PUSH_CURRENT();
                    RENEW_TOKEN(token_type::NONE);
                    break;

                case '\\':
                    escaped = true;
                    break;

                default:
                    if (current.type != token_type::STRING) {
                        current.begin = i;
                        current.type = token_type::STRING;
                    }
                    break;
                }

#undef RENEW_TOKEN
#undef PUSH_CURRENT
            }

            if (current.type != token_type::NONE) {
                current.end = commandline.size();
                tokens.push_back(current);
            }

            current.type = token_type::END_STMT;
            tokens.push_back(current);

            return tokens;
        }

        bool is_assignment(std::string const &commandline, token const &tk) {
            auto beg = commandline.begin() + tk.begin,
                 en = commandline.begin() + tk.end;
            auto pos_eq = std::find(beg, en, '=');
            if (pos_eq == en) {
                return false;
            }

            if (!std::isalpha(commandline[tk.begin]) &&
                commandline[tk.begin] != '_')
                return false;
            for (auto itr = beg + 1; itr != pos_eq; ++itr) {
                if (!std::isalnum(*itr) && *itr != '_') return false;
            }
            return true;
        }

        command_chain *parse(std::string const &commandline,
                             std::vector<token> const &tokens) {
            command_chain chain_first;
            command_chain *current = &chain_first;

            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i].type == token_type::END_STMT) continue;

                current->next = new command_chain;
                current = current->next;

                if (is_assignment(commandline, tokens[i])) {
                    auto beg = commandline.begin() + tokens[i].begin,
                         en = commandline.begin() + tokens[i].end;
                    auto eq = std::find(beg, en, '=');

                    current->type = command_type::ASSIGN;
                    single_statement *stmt = new assignment_statement(
                        std::string(beg, eq), std::string(eq + 1, en));
                    current->stmt = stmt;
                } else {
                    current->type = command_type::COMMAND;
                    command_statement *stmt = new command_statement;
                    current->stmt = stmt;

                    for (; i < tokens.size(); ++i) {
                        if (tokens[i].type != token_type::STRING) break;

                        stmt->push_arg(
                            std::string(commandline.begin() + tokens[i].begin,
                                        commandline.begin() + tokens[i].end));
                    }
                }
            }
            return chain_first.next;
        }
    } // namespace

    command_chain *parse_command_line(std::string commandline) {
        return parse(commandline, tokenize(commandline));
    }

    void command_chain_clean_up(command_chain *obj) {
        command_chain *next = obj;
        while (next) {
            delete next->stmt;
            command_chain *current = next;
            next = next->next;
            delete current;
        }
    }

    int assignment_statement::execute() { return 0; }

    int command_statement::execute() { return command_execute(command_line); }
} // namespace ben
