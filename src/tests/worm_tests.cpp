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

//! The worm as a class: rezzed under a body, ticked, standing still, counting.

#include <worm.hpp>

#include <testing/testing.hpp>

TEST_CASE(a_worm_that_loads_stands_still_and_counts_its_ticks)
{
    TglCreatureDesc desc{};
    desc.creature_id = 42u;
    desc.eye_count = 2u;
    desc.ear_count = 2u;
    WormLib::Worm worm{desc};
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
    // The Grid zeroed the actions; a worm with nothing to say leaves them so.
    TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.0f);
    TEST_CHECK_EQUAL(actions.desired_turn_rate, 0.0f);
    TEST_CHECK_EQUAL(actions.vocalisation_strength, 0.0f);

    senses.tick = 1001u;
    worm.tick(senses, actions);
    TEST_CHECK_EQUAL(worm.ticksSeen(), 2u);
    TEST_CHECK_EQUAL(worm.lastTick(), 1001u);
}

int main()
{
    return static_cast<int>(TestingLib::runAll());
}
