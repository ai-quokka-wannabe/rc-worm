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

#include <tgl/tgl_program_abi.h>

#include <cstdint>

/*!
    The worm, as the Grid sees it: one creature behind one opaque handle, ticked by the Grid's own
    thread. Vanilla C++20 - nothing here knows a window exists. What the worm senses and what it
    intends cross to the panel (when there is one) through a seam of plain structs; this class is
    the tick's side of that seam.

    Etape 2: the worm that loads. It stands still - every answer is the zeroed default the Grid
    hands it - and counts what it was told, so a test can see that it was ticked.
*/
namespace WormLib
{

    class Worm {
    public:
        //! Rezzed under this body: the Grid decided the senses; the worm remembers what it was given.
        explicit Worm(const TglCreatureDesc& desc) noexcept;

        /*!
            One tick: the senses in, the actions out. Never blocks, never throws. The Grid zeroes
            `actions` before the call; a worm with nothing to say leaves them so, and stands.
        */
        void tick(const TglSenses& senses, TglActions& actions) noexcept;

        [[nodiscard]] std::uint64_t creatureId() const noexcept
        {
            return m_creature_id;
        }

        //! The senses the Grid gave this body: how many eyes and ears arrive each tick.
        [[nodiscard]] std::uint32_t eyeCount() const noexcept
        {
            return m_eye_count;
        }

        [[nodiscard]] std::uint32_t earCount() const noexcept
        {
            return m_ear_count;
        }

        //! How many ticks this worm has been given since it was rezzed.
        [[nodiscard]] std::uint64_t ticksSeen() const noexcept
        {
            return m_ticks_seen;
        }

        //! The Grid's tick counter as of the last tick, for lining a log up with the world's.
        [[nodiscard]] std::uint64_t lastTick() const noexcept
        {
            return m_last_tick;
        }

    private:
        std::uint64_t m_creature_id{0u};
        std::uint32_t m_eye_count{0u};
        std::uint32_t m_ear_count{0u};
        std::uint64_t m_ticks_seen{0u};
        std::uint64_t m_last_tick{0u};
    };

} // namespace WormLib
