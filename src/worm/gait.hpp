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
    The gait: which joint bends when. It is the creature's, not the world's - the world holds a
    servo at every pivot to the angle it is asked for and honours the body's declared swing and
    torque, and everything else about walking is decided here. The owner's rulings: the worm
    undulates, the undulation propels, and the worm is a robot whose pivots are servo motors.

    This body's gait is lateral undulation, the limbless standard: a travelling wave of joint
    angles running from head to tail. The wave's crest moves tailward as the phase advances, so
    the body's runners push the floor backwards and the body goes forward; run the phase the
    other way and it backs up. The User's word - the panel's forward and turn - is not a speed
    and not a heading: it is how hard the wave runs and which way the body is bent, and how far
    the body gets is the floor's answer, reported by the senses, never assumed here.

    Vanilla C++20, no Qt: a gait is arithmetic, and it must be the same arithmetic every time
    for the same words, because the world replays a life from what it was asked.
*/
namespace WormLib
{

    /*!
        The wave's length along the body, in segments: four, so an eight-segment worm carries
        two waves - the proportion of a lateral undulator, about one wavelength to a body length.
    */
    inline constexpr float GAIT_WAVELENGTH_SEGMENTS{4.0f};

    /*!
        The wave's frequency at full forward, cycles per second. A worm of this length at this
        frequency runs its wave along itself at about a metre a second of phase speed; the body
        goes some fraction of that, as the runners allow. A fact about this worm's muscles,
        chosen for what a slow undulator is, not for a speed.
    */
    inline constexpr float GAIT_FREQUENCY_HZ{0.45f};

    /*!
        The share of the way the amplitude and the bias move towards the User's word each tick:
        a launch swells the wave over half a second, a turn bends the body over the same, and a
        resting worm relaxes - it does not hold a frozen wave. Muscles ramp; they do not snap.
    */
    inline constexpr float GAIT_RISE{0.15f};

    //! How much of the declared swing a full turn bends every joint the same way, on top of the wave.
    inline constexpr float GAIT_TURN_SHARE{0.45f};

    //! The gait's state: where the wave stands, and how much of it the muscles are giving.
    class Gait {
    public:
        //! For a body with this many joints (segments less one) and the Grid's bound on their swing.
        Gait(std::uint32_t joint_count, float max_joint_angle) noexcept;

        /*!
            One tick: the User's forward and turn as fractions in [-1, 1] of the body's own
            bounds, `dt` the tick's seconds. Fills `targets` with the angle each servo is asked
            to hold - the wave at every joint, the bend on top - and advances the wave.
        */
        void tick(float forward_fraction, float turn_fraction, float dt, float (&targets)[TGL_SEGMENTS_MAX - 1u]) noexcept;

        [[nodiscard]] float phase() const noexcept
        {
            return m_phase;
        }

        [[nodiscard]] float amplitude() const noexcept
        {
            return m_amplitude;
        }

        [[nodiscard]] float bias() const noexcept
        {
            return m_bias;
        }

    private:
        std::uint32_t m_joint_count;
        float m_max_joint_angle;
        float m_phase{0.0f};
        float m_amplitude{0.0f};
        float m_bias{0.0f};
    };

} // namespace WormLib
