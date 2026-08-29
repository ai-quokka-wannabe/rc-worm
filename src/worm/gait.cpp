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

#include "gait.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace WormLib
{

    Gait::Gait(const std::uint32_t joint_count, const float max_joint_angle) noexcept :
        m_joint_count(std::min(joint_count, TGL_SEGMENTS_MAX - 1u)),
        m_max_joint_angle(std::max(max_joint_angle, 0.0f))
    {
    }

    void Gait::tick(const float forward_fraction, const float turn_fraction, const float dt, float (&targets)[TGL_SEGMENTS_MAX - 1u]) noexcept
    {
        const float forward{std::clamp(forward_fraction, -1.0f, 1.0f)};
        const float turn{std::clamp(turn_fraction, -1.0f, 1.0f)};

        // The wave runs as hard as the User asks, the other way for reverse, and stands when the
        // word is silence. Its phase is kept in [0, 2 pi) so a long life never loses precision.
        const float advance{2.0f * std::numbers::pi_v<float> * GAIT_FREQUENCY_HZ * forward * dt};
        m_phase = std::fmod(m_phase + advance + 2.0f * std::numbers::pi_v<float>, 2.0f * std::numbers::pi_v<float>);

        // The muscles ramp: the amplitude towards the full swing when asked to move, towards
        // nothing at rest; the bend towards the turn's share of the swing.
        const float wanted_amplitude{m_max_joint_angle * std::fabs(forward)};
        m_amplitude += (wanted_amplitude - m_amplitude) * GAIT_RISE;
        const float wanted_bias{m_max_joint_angle * GAIT_TURN_SHARE * turn};
        m_bias += (wanted_bias - m_bias) * GAIT_RISE;

        for (std::uint32_t joint{0u}; joint < TGL_SEGMENTS_MAX - 1u; ++joint) {
            if (joint >= m_joint_count) {
                targets[joint] = 0.0f;
                continue;
            }
            // Each joint lags the one before by its share of the wavelength, so the crest travels
            // tailward as the phase advances - the way the runners need it to.
            const float lag{static_cast<float>(joint) * 2.0f * std::numbers::pi_v<float> / GAIT_WAVELENGTH_SEGMENTS};
            const float asked{(m_amplitude * std::sin(m_phase - lag)) + m_bias};
            // Never past the declared swing: the sum of a full wave and a full bend would be.
            targets[joint] = std::clamp(asked, -m_max_joint_angle, m_max_joint_angle);
        }
    }

} // namespace WormLib
