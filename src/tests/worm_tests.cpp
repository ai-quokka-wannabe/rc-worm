/*
    Copyright (C) 2026 The AiQuokkaWannabe Authors https://github.com/ai-quokka-wannabe/rc-worm

    This program is free software: you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation, either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.
    If not, see https://www.gnu.org/licenses/.
*/

//! The worm as a class: rezzed under a body, ticked, standing still, counting - and its gait.

#include <gait.hpp>
#include <worm.hpp>

#include <testing/testing.hpp>

#include <cmath>
#include <cstdint>
#include <numbers>

TEST_CASE(a_worm_that_loads_stands_still_and_counts_its_ticks)
{
    TglCreatureDesc desc{};
    desc.creature_id = 42u;
    desc.eye_count = 2u;
    desc.ear_count = 2u;
    WormLib::Worm worm{desc, 0.03125f};
    TEST_CHECK_EQUAL(worm.body().nominal_dt_seconds, 0.03125f);
    TEST_CHECK_EQUAL(worm.creatureId(), 42u);
    TEST_CHECK_EQUAL(worm.eyeCount(), 2u);
    TEST_CHECK_EQUAL(worm.earCount(), 2u);
    TEST_CHECK_EQUAL(worm.ticksSeen(), 0u);

    TglSenses senses{};
    senses.tick = 1000u;
    senses.dt_seconds = 0.03125f;
    TglActions actions{};
    worm.tick(senses, actions);
    TEST_CHECK_EQUAL(worm.ticksSeen(), 1u);
    TEST_CHECK_EQUAL(worm.lastTick(), 1000u);
    // The Grid zeroed the actions; a worm nobody steers leaves them so and stands - no servo
    // asked to bend from rest.
    TEST_CHECK(worm.lastApplied() == WormLib::Applied::Braked);
    TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.0f);
    TEST_CHECK_EQUAL(actions.desired_turn_rate, 0.0f);
    TEST_CHECK_EQUAL(actions.vocalisation_strength, 0.0f);
    for (const float target : actions.joint_targets) {
        TEST_CHECK_EQUAL(target, 0.0f);
    }

    senses.tick = 1001u;
    worm.tick(senses, actions);
    TEST_CHECK_EQUAL(worm.ticksSeen(), 2u);
    TEST_CHECK_EQUAL(worm.lastTick(), 1001u);
}

TEST_CASE(the_gait_stays_within_the_swing_keeps_its_phase_bounded_and_is_the_same_every_time)
{
    // The guarantees gait.hpp states, held over a long life of every word at its extremes and
    // beyond them: never past the declared swing, nothing asked of a joint the body does not
    // have, the phase kept in [0, 2 pi), and the same words giving the same angles every time.
    constexpr float SWING{0.9f};
    constexpr float DT{0.03125f};
    constexpr std::uint32_t JOINTS{5u};
    WormLib::Gait one{JOINTS, SWING};
    WormLib::Gait two{JOINTS, SWING};
    const float words[][2]{{1.0f, 1.0f}, {-1.0f, -1.0f}, {3.0f, -7.0f}, {0.5f, 0.0f}, {0.0f, 0.0f}};
    for (std::uint32_t tick{0u}; tick < 20000u; ++tick) {
        const float* const word{words[(tick / 400u) % 5u]};
        float first[TGL_SEGMENTS_MAX - 1u]{};
        float second[TGL_SEGMENTS_MAX - 1u]{};
        one.tick(word[0], word[1], DT, first);
        two.tick(word[0], word[1], DT, second);
        for (std::uint32_t joint{0u}; joint < TGL_SEGMENTS_MAX - 1u; ++joint) {
            TEST_CHECK(std::fabs(first[joint]) <= SWING);
            TEST_CHECK_EQUAL(first[joint], second[joint]);
            if (joint >= JOINTS) {
                TEST_CHECK_EQUAL(first[joint], 0.0f);
            }
        }
        TEST_CHECK((one.phase() >= 0.0f) && (one.phase() < 2.0f * std::numbers::pi_v<float>));
    }

    // A body with no servos is asked for nothing, whatever the word.
    WormLib::Gait lone{0u, 0.0f};
    float targets[TGL_SEGMENTS_MAX - 1u]{};
    lone.tick(1.0f, 1.0f, DT, targets);
    for (const float target : targets) {
        TEST_CHECK_EQUAL(target, 0.0f);
    }
}

int main()
{
    return static_cast<int>(TestingLib::runAll());
}
