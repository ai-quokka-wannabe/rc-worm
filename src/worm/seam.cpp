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

#include "seam.hpp"

#include <algorithm>
#include <cstring>

namespace WormLib
{

    namespace
    {

        void copy3(float (&to)[3], const float (&from)[3]) noexcept
        {
            to[0] = from[0];
            to[1] = from[1];
            to[2] = from[2];
        }

    } // namespace

    BodySnapshot snapshotBody(const TglCreatureDesc& desc, const float nominal_dt_seconds) noexcept
    {
        BodySnapshot body{};
        body.creature_id = desc.creature_id;
        body.max_contact_count = desc.max_contact_count;
        body.max_forward_speed = desc.max_forward_speed;
        body.max_turn_rate = desc.max_turn_rate;
        body.max_vocalisation_strength = desc.max_vocalisation_strength;
        body.nominal_dt_seconds = nominal_dt_seconds;

        body.eye_count = std::min(desc.eye_count, SEAM_EYES_MAX);
        body.eyes_dropped = desc.eye_count - body.eye_count;
        for (std::uint32_t index{0u}; (index < body.eye_count) && (desc.eyes != nullptr); ++index) {
            const TglEyeDesc& from{desc.eyes[index]};
            EyeStation& to{body.eyes[index]};
            copy3(to.position, from.position);
            to.channels = from.channels;
            to.quantisation_bits = from.quantisation_bits;
            to.sample_count = std::min(from.sample_count, SEAM_EYE_SAMPLES_MAX);
            to.samples_dropped = from.sample_count - to.sample_count;
            if (from.sample_directions != nullptr) {
                std::memcpy(to.directions, from.sample_directions, sizeof(float) * 3u * to.sample_count);
            }
            if (from.sample_acceptance_angles != nullptr) {
                std::memcpy(to.acceptance_radians, from.sample_acceptance_angles, sizeof(float) * to.sample_count);
            }
        }

        body.ear_count = std::min(desc.ear_count, SEAM_EARS_MAX);
        body.ears_dropped = desc.ear_count - body.ear_count;
        for (std::uint32_t index{0u}; (index < body.ear_count) && (desc.ears != nullptr); ++index) {
            const TglEarDesc& from{desc.ears[index]};
            EarStation& to{body.ears[index]};
            copy3(to.position, from.position);
            to.bin_seconds = from.bin_seconds;
            to.band_count = std::min(from.band_count, SEAM_EAR_BANDS_MAX);
            to.bands_dropped = from.band_count - to.band_count;
            to.bin_count = std::min(from.bin_count, SEAM_EAR_BINS_MAX);
            to.bins_dropped = from.bin_count - to.bin_count;
            if (from.band_edges_hz != nullptr) {
                std::memcpy(to.band_edges_hz, from.band_edges_hz, sizeof(float) * (to.band_count + 1u));
            }
        }
        return body;
    }

    void snapshotSenses(const TglSenses& senses, SensesSnapshot& out) noexcept
    {
        out.tick = senses.tick;
        out.dt_seconds = senses.dt_seconds;
        out.body_forward_speed = senses.body_forward_speed;
        out.body_vertical_speed = senses.body_vertical_speed;
        out.body_turn_rate = senses.body_turn_rate;
        copy3(out.specific_force, senses.specific_force);
        copy3(out.angular_velocity, senses.angular_velocity);
        out.irradiance = senses.irradiance;

        out.eye_count = std::min(senses.eye_count, SEAM_EYES_MAX);
        out.eyes_dropped = senses.eye_count - out.eye_count;
        for (std::uint32_t index{0u}; index < out.eye_count; ++index) {
            EyeSnapshot& to{out.eyes[index]};
            if (senses.eyes == nullptr) {
                to = EyeSnapshot{};
                continue;
            }
            const TglEyeView& from{senses.eyes[index]};
            to.channels = std::min(from.channels, SEAM_EYE_CHANNELS_MAX);
            // A sample is copied whole or not at all: channels beyond the cap drop the sample.
            to.sample_count = (from.channels > SEAM_EYE_CHANNELS_MAX) ? 0u : std::min(from.sample_count, SEAM_EYE_SAMPLES_MAX);
            to.samples_dropped = from.sample_count - to.sample_count;
            if ((from.samples != nullptr) && (to.sample_count > 0u)) {
                std::memcpy(to.samples, from.samples, sizeof(float) * to.sample_count * to.channels);
            }
        }

        out.ear_count = std::min(senses.ear_count, SEAM_EARS_MAX);
        out.ears_dropped = senses.ear_count - out.ear_count;
        for (std::uint32_t index{0u}; index < out.ear_count; ++index) {
            EarSnapshot& to{out.ears[index]};
            if (senses.ears == nullptr) {
                to = EarSnapshot{};
                continue;
            }
            const TglEarView& from{senses.ears[index]};
            to.band_count = std::min(from.band_count, SEAM_EAR_BANDS_MAX);
            to.bands_dropped = from.band_count - to.band_count;
            to.bin_count = std::min(from.bin_count, SEAM_EAR_BINS_MAX);
            to.bins_dropped = from.bin_count - to.bin_count;
            // Band-major on both sides; a band is one contiguous run of bins.
            for (std::uint32_t band{0u}; (band < to.band_count) && (from.energy != nullptr); ++band) {
                std::memcpy(&to.energy[band * SEAM_EAR_BINS_MAX], &from.energy[band * from.bin_count], sizeof(float) * to.bin_count);
            }
            to.arrival_count = std::min(from.arrival_count, TGL_EAR_ARRIVALS_MAX);
            if ((from.arrivals != nullptr) && (to.arrival_count > 0u)) {
                std::memcpy(to.arrivals, from.arrivals, sizeof(TglArrival) * to.arrival_count);
            }
        }

        out.contact_count = (senses.contacts == nullptr) ? 0u : std::min(senses.contact_count, SEAM_CONTACTS_MAX);
        out.contacts_dropped = senses.contact_count - out.contact_count;
        if (out.contact_count > 0u) {
            std::memcpy(out.contacts, senses.contacts, sizeof(TglContact) * out.contact_count);
        }
    }

    void Mailbox::publishSenses(const SensesSnapshot& senses) noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        m_senses = senses;
        ++m_senses_generation;
        m_senses.generation = m_senses_generation;
    }

    bool Mailbox::takeSenses(SensesSnapshot& out, std::uint64_t& seen) const noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        if (m_senses_generation <= seen) {
            return false;
        }
        out = m_senses;
        seen = m_senses_generation;
        return true;
    }

    std::uint64_t Mailbox::sensesGeneration() const noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        return m_senses_generation;
    }

    void Mailbox::offerIntent(const Intent& intent) noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        m_intent = intent;
        ++m_intent_generation;
        m_intent.generation = m_intent_generation;
        m_call_latched = std::max(m_call_latched, intent.vocalisation);
    }

    std::optional<Intent> Mailbox::takeIntent() noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        if (m_intent_generation <= m_intent_taken) {
            return std::nullopt;
        }
        m_intent_taken = m_intent_generation;
        Intent taken{m_intent};
        taken.vocalisation = m_call_latched;
        m_call_latched = 0.0f;
        return taken;
    }

    std::uint64_t Mailbox::intentGeneration() const noexcept
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        return m_intent_generation;
    }

} // namespace WormLib
