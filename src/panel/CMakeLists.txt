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

# The panel: the one place Qt lives. A static library the Program links, built on every kit; the
# window and the Qt thread behind a header no Qt type crosses. qt_standard_project_setup() turns
# moc on for THIS directory only, so the worm beside it is never scanned.

qt_standard_project_setup()

qt_add_library(panel STATIC
    panel.cpp
    window.cpp
    window.hpp
)

target_include_directories(panel PUBLIC include)
# The seam's structs are the worm's; the panel reads them, the worm never reads the panel.
# CMAKE_DL_LIBS for dladdr on Linux: the panel asks which directory it was loaded from, so a
# Qt deployed beside the worm is on Qt's library path before any window is asked for.
target_link_libraries(panel
    PUBLIC worm
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ${CMAKE_DL_LIBS}
)
target_compile_features(panel PUBLIC cxx_std_20)
set_target_properties(panel PROPERTIES POSITION_INDEPENDENT_CODE ON)

message(STATUS "Panel: Qt ${Qt6_VERSION} from ${Qt6_DIR}")
