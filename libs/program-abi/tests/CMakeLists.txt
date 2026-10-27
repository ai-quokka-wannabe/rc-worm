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

# The header compiled as C17 and as C++20, once each: every static assertion in it runs against the
# very compilers this Program is built with. A vendored header that no longer matched either would
# fail here, before any Program was linked against it.
add_library(program_abi_compiles_as_c OBJECT compiles_as_c.c)
target_link_libraries(program_abi_compiles_as_c PRIVATE program_abi)
set_target_properties(program_abi_compiles_as_c PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED ON)

add_library(program_abi_compiles_as_cxx OBJECT compiles_as_cxx.cpp)
target_link_libraries(program_abi_compiles_as_cxx PRIVATE program_abi)
target_compile_features(program_abi_compiles_as_cxx PRIVATE cxx_std_20)
