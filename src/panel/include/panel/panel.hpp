/*
    Copyright (C) 2026 Matej Gomboc https://github.com/ai-quokka-wannabe/rc-worm

    This program is free software: you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation, either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.
    If not, see https://www.gnu.org/licenses/.
*/

#pragma once

/*!
    The panel's face to the rest of the worm: plain C++20, no Qt type crosses it. Everything Qt is
    behind this header, in the panel's own sources, which are the only files in this repository
    allowed to include one.
*/
namespace PanelLib
{

    //! The Qt the panel was built against, as its own runtime reports it - the kit, proven.
    [[nodiscard]] const char* qtVersion() noexcept;

} // namespace PanelLib
