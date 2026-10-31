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

# The gait figure: a small executable that ticks the worm's own gait (the `worm` library, not a
# second model of it) and writes docs/figures/gait.svg by hand. Never installed - it is no part
# of the Program - but built with everything else so it cannot rot, and run once as a test.
# Regenerate: build the target, then run it with the figure's path:
#   build/<preset>/bin/<config>/gait_figure docs/figures/gait.svg

add_executable(gait_figure gait_figure.cpp)
target_link_libraries(gait_figure PRIVATE worm)
target_compile_features(gait_figure PRIVATE cxx_std_20)
add_test(NAME gait_figure COMMAND gait_figure "${CMAKE_CURRENT_BINARY_DIR}/gait.svg")
