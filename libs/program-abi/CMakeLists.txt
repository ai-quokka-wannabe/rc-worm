# Copyright (C) 2026 The AiQuokkaWannabe Authors https://github.com/ai-quokka-wannabe/rc-worm
#
# This program is free software: you can redistribute it and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program.
# If not, see https://www.gnu.org/licenses/.

# The Program ABI, vendored verbatim from the flagship (tron-grid-lite/libs/program-abi): the header
# and its fingerprint. The flagship moves it; this repository re-vendors and never edits. A header
# and nothing else, so an INTERFACE target with no compile features - the file compiles as C or C++
# under any standard, and a target that pinned one would quietly stop being true of the other.

add_library(program_abi INTERFACE)

target_include_directories(program_abi
    INTERFACE include/
)

add_subdirectory(tests)
