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

#include "worm.hpp"

namespace WormLib
{

    Worm::Worm(const TglCreatureDesc& desc) noexcept :
        m_creature_id(desc.creature_id),
        m_eye_count(desc.eye_count),
        m_ear_count(desc.ear_count)
    {
    }

    void Worm::tick(const TglSenses& senses, TglActions& actions) noexcept
    {
        ++m_ticks_seen;
        m_last_tick = senses.tick;

        // Etape 2: the worm stands. The Grid zeroed the actions; they stay zeroed. The panel, when
        // it arrives, is what puts a number here - and nothing else ever will.
        (void)actions;
    }

} // namespace WormLib
