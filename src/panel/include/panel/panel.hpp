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

    One Qt thread per library - a process may hold one QApplication, and this library is not the
    process - started in library_init and joined in library_shutdown; one window per rezzed
    creature, opened at rez and closed, to completion, at derez. None of these is ever called from
    program_tick, which never waits for a window.
*/
namespace WormLib
{
    class Mailbox;
    struct BodySnapshot;
} // namespace WormLib

namespace PanelLib
{

    //! The Qt the panel was built against, as its own runtime reports it - the kit, proven.
    [[nodiscard]] const char* qtVersion() noexcept;

    /*!
        Starts the Qt thread and waits until its QApplication is up. False when it cannot be: no
        display to draw into (a headless Linux without an X server, unless QT_QPA_PLATFORM says
        otherwise), or the thread refused to start. Idempotent; a second call answers whether it
        is running.
    */
    [[nodiscard]] bool start() noexcept;

    [[nodiscard]] bool running() noexcept;

    //! A window, opaque to everything outside the panel.
    struct Window;

    /*!
        Opens a window steering the creature behind `mailbox`, drawn from `body`; blocks until the
        window exists on the Qt thread. Null when the panel is not running. The window reads
        `mailbox` until close() - the caller keeps the mailbox alive that long.
    */
    [[nodiscard]] Window* open(WormLib::Mailbox& mailbox, const WormLib::BodySnapshot& body) noexcept;

    //! Closes and destroys the window, blocking until it no longer touches its mailbox. Null is fine.
    void close(Window* window) noexcept;

    //! Quits the Qt thread and joins it. Every window must be closed first. Idempotent.
    void stop() noexcept;

} // namespace PanelLib
