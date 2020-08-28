# Interactive binary viewer.
# Copyright (C) 2020  Koki Fukuda
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.17)
project(ben
  VERSION 0.0.1
  DESCRIPTION "Interactive binary viewer."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED Yes)

find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)

add_executable(ben)
target_include_directories(ben PRIVATE ${READLINE_INCLUDE_DIRS})
target_link_libraries(ben PRIVATE ${READLINE_LIBRARIES})
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" AND UNIX)
  target_compile_options(ben PRIVATE -fsanitize=leak -fsanitize=address)
  target_link_options(ben PRIVATE -fsanitize=leak -fsanitize=address)
endif()

add_subdirectory(src)
