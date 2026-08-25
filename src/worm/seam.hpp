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
#include <mutex>
#include <optional>

/*!
    The seam between the tick and the panel: plain structs, copied whole under a mutex held for
    the copy and nothing else. `program_tick` runs on the Grid's thread and must never wait for a
    window; the panel runs on the Qt thread and must never touch memory the Grid lent for one call.
    So the tick copies what the Grid lent into a snapshot of its own, publishes the snapshot, and
    takes the newest intent; the panel takes the newest snapshot and offers intents. Neither side
    ever blocks on the other, and no pointer crosses.

    Fixed capacities, because a snapshot is copied every tick and must not allocate: a body richer
    than these is not silently trimmed - every dropped sample, band, bin or contact is COUNTED in
    the snapshot, and the panel says so by name. Vanilla C++20; docs/PANEL.md carries the rules.
*/
namespace WormLib
{

    inline constexpr std::uint32_t SEAM_EYES_MAX{4u};
    inline constexpr std::uint32_t SEAM_EYE_SAMPLES_MAX{256u};
    inline constexpr std::uint32_t SEAM_EYE_CHANNELS_MAX{3u};
    inline constexpr std::uint32_t SEAM_EARS_MAX{4u};
    inline constexpr std::uint32_t SEAM_EAR_BANDS_MAX{8u};
    inline constexpr std::uint32_t SEAM_EAR_BINS_MAX{128u};
    inline constexpr std::uint32_t SEAM_CONTACTS_MAX{32u};

    /*!
        The panel's silence rule, in ticks: a tick that finds no new intent repeats the last one for
        this many ticks, then brakes. Four ticks is an eighth of a second at the Grid's 32 Hz - long
        enough to ride out the panel's poll landing between two ticks, short enough that a panel
        that stalled or died stops the worm before it has gone anywhere. The wire's own rule is the
        same shape (LNK_ACTIONS_REPEAT_TICKS); this one is the panel's.
    */
    inline constexpr std::uint32_t PANEL_REPEAT_TICKS{4u};

    //! One eye as the Grid described it at rez: where it sits and where each sample looks.
    struct EyeStation {
        float position[3];
        float directions[SEAM_EYE_SAMPLES_MAX * 3u];
        float acceptance_radians[SEAM_EYE_SAMPLES_MAX];
        std::uint32_t sample_count;
        std::uint32_t channels;
        std::uint32_t quantisation_bits;
        std::uint32_t samples_dropped;
    };

    //! One ear as the Grid described it: where it sits and how it bins time and frequency.
    struct EarStation {
        float position[3];
        float band_edges_hz[SEAM_EAR_BANDS_MAX + 1u];
        std::uint32_t band_count;
        std::uint32_t bin_count;
        float bin_seconds;
        std::uint32_t bands_dropped;
        std::uint32_t bins_dropped;
    };

    //! The body the worm was given, copied once at rez so the panel can read it for a lifetime.
    struct BodySnapshot {
        std::uint64_t creature_id;
        EyeStation eyes[SEAM_EYES_MAX];
        EarStation ears[SEAM_EARS_MAX];
        std::uint32_t eye_count;
        std::uint32_t ear_count;
        std::uint32_t eyes_dropped;
        std::uint32_t ears_dropped;
        std::uint32_t max_contact_count;
        float max_forward_speed;
        float max_turn_rate;
        float max_vocalisation_strength;
        float nominal_dt_seconds;
    };

    [[nodiscard]] BodySnapshot snapshotBody(const TglCreatureDesc& desc, float nominal_dt_seconds) noexcept;

    struct EyeSnapshot {
        float samples[SEAM_EYE_SAMPLES_MAX * SEAM_EYE_CHANNELS_MAX];
        std::uint32_t sample_count;
        std::uint32_t channels;
        std::uint32_t samples_dropped;
    };

    struct EarSnapshot {
        float energy[SEAM_EAR_BANDS_MAX * SEAM_EAR_BINS_MAX];
        TglArrival arrivals[TGL_EAR_ARRIVALS_MAX];
        std::uint32_t band_count;
        std::uint32_t bin_count;
        std::uint32_t arrival_count;
        std::uint32_t bands_dropped;
        std::uint32_t bins_dropped;
    };

    //! Everything the worm sensed this tick, owned rather than borrowed. Some thirty kilobytes.
    struct SensesSnapshot {
        std::uint64_t tick;
        //! Stamped by the mailbox on publish, counting from one; zero is "never published".
        std::uint64_t generation;
        EyeSnapshot eyes[SEAM_EYES_MAX];
        EarSnapshot ears[SEAM_EARS_MAX];
        TglContact contacts[SEAM_CONTACTS_MAX];
        std::uint32_t eye_count;
        std::uint32_t ear_count;
        std::uint32_t contact_count;
        std::uint32_t eyes_dropped;
        std::uint32_t ears_dropped;
        std::uint32_t contacts_dropped;
        float dt_seconds;
        float body_forward_speed;
        float body_vertical_speed;
        float body_turn_rate;
        float specific_force[3];
        float angular_velocity[3];
        float irradiance;
    };

    //! Copies what the Grid lent into `out`, counting what did not fit. Never touches `generation`.
    void snapshotSenses(const TglSenses& senses, SensesSnapshot& out) noexcept;

    //! What the User asks of the body: the three members of TglActions, and when they were asked.
    struct Intent {
        float forward_speed;
        float turn_rate;
        float vocalisation;
        //! Stamped by the mailbox on offer, counting from one.
        std::uint64_t generation;
    };

    /*!
        One mailbox per creature, one slot each way. Senses: latest wins, the panel takes only what
        is newer than it has seen. Intent: forward and turn are latest-wins too, but a call is
        LATCHED - the loudest vocalisation offered since the tick last took is what the tick gets,
        so a call from the panel (which polls slower than the Grid ticks) can never be overwritten
        by the panel's own next poll before a tick has heard it. A call is one burst per tick, so
        it is delivered exactly once.
    */
    class Mailbox {
    public:
        //! The tick's side: publish this tick's senses; the newest generation replaces the last.
        void publishSenses(const SensesSnapshot& senses) noexcept;

        /*!
            The panel's side: copies the snapshot into `out` if one newer than `seen` exists, and
            advances `seen` to it. False, and nothing touched, otherwise.
        */
        [[nodiscard]] bool takeSenses(SensesSnapshot& out, std::uint64_t& seen) const noexcept;

        [[nodiscard]] std::uint64_t sensesGeneration() const noexcept;

        //! The panel's side: this is what the User asks now.
        void offerIntent(const Intent& intent) noexcept;

        //! The tick's side: the newest intent since the last take, or nothing if none arrived.
        [[nodiscard]] std::optional<Intent> takeIntent() noexcept;

        [[nodiscard]] std::uint64_t intentGeneration() const noexcept;

    private:
        mutable std::mutex m_mutex;
        SensesSnapshot m_senses{};
        std::uint64_t m_senses_generation{0u};
        Intent m_intent{};
        std::uint64_t m_intent_generation{0u};
        std::uint64_t m_intent_taken{0u};
        float m_call_latched{0.0f};
    };

} // namespace WormLib
