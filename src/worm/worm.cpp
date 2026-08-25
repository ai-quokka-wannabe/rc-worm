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

    Worm::Worm(const TglCreatureDesc& desc, const float nominal_dt_seconds) noexcept :
        m_body(snapshotBody(desc, nominal_dt_seconds))
    {
    }

    void Worm::tick(const TglSenses& senses, TglActions& actions) noexcept
    {
        ++m_ticks_seen;
        m_last_tick = senses.tick;

        // What the Grid lent is copied whole before anything else looks at it: the panel reads
        // the copy, on its own thread, long after this call has returned.
        snapshotSenses(senses, m_snapshot);
        m_mailbox.publishSenses(m_snapshot);

        // The User's word, the last word repeated, or silence: the panel's silence rule.
        if (const std::optional<Intent> fresh{m_mailbox.takeIntent()}) {
            m_last_intent = *fresh;
            m_repeat_budget = PANEL_REPEAT_TICKS;
            m_last_applied = Applied::Fresh;
        } else if (m_repeat_budget > 0u) {
            --m_repeat_budget;
            // A call is one burst per tick; a repeat repeats the motion, never the voice.
            m_last_intent.vocalisation = 0.0f;
            m_last_applied = Applied::Repeated;
        } else {
            m_last_intent = Intent{};
            m_last_applied = Applied::Braked;
        }

        // The Grid zeroed the actions; a braked worm leaves them so and stands.
        actions.desired_forward_speed = m_last_intent.forward_speed;
        actions.desired_turn_rate = m_last_intent.turn_rate;
        actions.vocalisation_strength = m_last_intent.vocalisation;
    }

} // namespace WormLib
