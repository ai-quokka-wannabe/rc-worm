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

if(RC_WORM_PANEL)
    # Found here rather than in the panel's own directory so the tests, a sibling, can name the
    # Qt runtime the library needs at load and link Qt Test for the panel's own tests.
    find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Widgets Test)
endif()

# The worm first: the panel reads its seam, the worm knows nothing of the panel.
add_subdirectory(worm)
if(RC_WORM_PANEL)
    add_subdirectory(panel)
endif()
add_subdirectory(tests)
