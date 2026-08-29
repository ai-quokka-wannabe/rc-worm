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

#include "gait.hpp"
#include "seam.hpp"

#include <tgl/tgl_program_abi.h>

#include <cstdint>

/*!
    The worm, as the Grid sees it: one creature behind one opaque handle, ticked by the Grid's own
    thread. Vanilla C++20 - nothing here knows a window exists. What the worm senses and what it
    intends cross to the panel (when there is one) through the seam's plain structs; this class is
    the tick's side of that seam.

    Etape 4: the User is the brain. Every tick the worm publishes what it sensed and answers with
    the newest intent the panel offered; a tick that finds no new intent repeats the last one for
    PANEL_REPEAT_TICKS and then brakes - the panel's silence, like the network's, is a repeat and
    then a stop. A worm nobody steers stands.
*/
namespace WormLib
{

    //! How a tick's answer came to be - the record a test (or a log) is owed.
    enum class Applied : std::uint8_t {
        Fresh, //!< The newest intent the panel offered, taken this tick.
        Repeated, //!< No new intent; the last one re-applied within PANEL_REPEAT_TICKS.
        Braked, //!< Silence past the budget, or nothing ever offered: zeroes, and the body stops.
    };

    class Worm {
    public:
        //! Rezzed under this body: the Grid decided the senses; the worm remembers what it was given.
        Worm(const TglCreatureDesc& desc, float nominal_dt_seconds) noexcept;

        /*!
            One tick: the senses in, the actions out. Never blocks, never throws. The Grid zeroes
            `actions` before the call; the worm writes the intent the User asked for, or repeats,
            or leaves them zeroed and stands.
        */
        void tick(const TglSenses& senses, TglActions& actions) noexcept;

        //! The seam: the panel's side takes senses from it and offers intents into it.
        [[nodiscard]] Mailbox& mailbox() noexcept
        {
            return m_mailbox;
        }

        //! The body the Grid described at rez, as the panel needs it to draw the senses.
        [[nodiscard]] const BodySnapshot& body() const noexcept
        {
            return m_body;
        }

        [[nodiscard]] std::uint64_t creatureId() const noexcept
        {
            return m_body.creature_id;
        }

        //! The senses the Grid gave this body: how many eyes and ears arrive each tick.
        [[nodiscard]] std::uint32_t eyeCount() const noexcept
        {
            return m_body.eye_count + m_body.eyes_dropped;
        }

        [[nodiscard]] std::uint32_t earCount() const noexcept
        {
            return m_body.ear_count + m_body.ears_dropped;
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

        //! How the last tick's answer came to be.
        [[nodiscard]] Applied lastApplied() const noexcept
        {
            return m_last_applied;
        }

        //! The gait as it stands: where the wave is, how much of it the muscles give.
        [[nodiscard]] const Gait& gait() const noexcept
        {
            return m_gait;
        }

        //! Ticks of repeat left before silence becomes a brake.
        [[nodiscard]] std::uint32_t repeatBudget() const noexcept
        {
            return m_repeat_budget;
        }

    private:
        BodySnapshot m_body;
        Mailbox m_mailbox;
        //! The worm's own gait: the wave its servos are asked to hold, from the User's word.
        Gait m_gait;
        //! The tick's own copy of what the Grid lent, kept as a member so the tick's stack stays small.
        SensesSnapshot m_snapshot{};
        Intent m_last_intent{};
        std::uint32_t m_repeat_budget{0u};
        Applied m_last_applied{Applied::Braked};
        std::uint64_t m_ticks_seen{0u};
        std::uint64_t m_last_tick{0u};
    };

} // namespace WormLib
