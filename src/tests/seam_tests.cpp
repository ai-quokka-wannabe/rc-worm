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

/*!
    The seam, deviceless: the mailbox's ordering (latest wins, nothing taken twice, a call latched
    until a tick hears it), the snapshot counting what did not fit, and the worm's silence rule -
    fresh, repeated for PANEL_REPEAT_TICKS, then braked - with the test playing the panel.
*/

#include <seam.hpp>
#include <worm.hpp>

#include <testing/testing.hpp>

#include <array>
#include <cstdint>
#include <thread>
#include <vector>

namespace
{

    //! The first body as the flagship describes it: two eyes of one sample, two ears of 4 x 64.
    struct FirstBody {
        std::array<float, 3> head_direction{0.0f, 0.0f, -1.0f};
        std::array<float, 3> tail_direction{0.0f, 0.0f, 1.0f};
        std::array<float, 1> acceptance{0.5235988f};
        std::array<float, 5> band_edges{2000.0f, 4500.0f, 7500.0f, 10500.0f, 13500.0f};
        std::array<float, 4> absorption{0.0f, 0.0f, 0.0f, 0.0f};
        std::array<TglEyeDesc, 2> eyes{};
        std::array<TglEarDesc, 2> ears{};
        TglCreatureDesc desc{};

        FirstBody()
        {
            eyes[0] = TglEyeDesc{.sample_directions = head_direction.data(),
                .sample_acceptance_angles = acceptance.data(),
                .position = {0.0f, 0.0f, -0.2f},
                .sample_count = 1u,
                .channels = 1u,
                .quantisation_bits = 0u};
            eyes[1] = TglEyeDesc{.sample_directions = tail_direction.data(),
                .sample_acceptance_angles = acceptance.data(),
                .position = {0.0f, 0.0f, 0.2f},
                .sample_count = 1u,
                .channels = 1u,
                .quantisation_bits = 0u};
            for (std::size_t index{0u}; index < 2u; ++index) {
                ears[index] = TglEarDesc{.band_edges_hz = band_edges.data(),
                    .air_absorption_db_per_km = absorption.data(),
                    .position = {0.0f, 0.0f, index == 0u ? -0.2f : 0.2f},
                    .band_count = 4u,
                    .bin_count = 64u,
                    .bin_seconds = 0.001f};
            }
            desc.creature_id = 256u;
            desc.eyes = eyes.data();
            desc.ears = ears.data();
            desc.eye_count = 2u;
            desc.ear_count = 2u;
            desc.max_contact_count = 4u;
            desc.max_forward_speed = 1.0f;
            desc.max_turn_rate = 1.5707964f;
            desc.max_vocalisation_strength = 1.0f;
        }
    };

    //! One tick's senses for that body, with a number in every place a panel would read.
    struct FirstSenses {
        std::array<float, 1> head_sample{0.75f};
        std::array<float, 1> tail_sample{0.25f};
        std::array<float, 4u * 64u> energy_a{};
        std::array<float, 4u * 64u> energy_b{};
        std::array<TglArrival, 2> arrivals{};
        std::array<TglEyeView, 2> eyes{};
        std::array<TglEarView, 2> ears{};
        std::array<TglContact, 2> contacts{};
        TglSenses senses{};

        explicit FirstSenses(const std::uint64_t tick)
        {
            energy_a[(1u * 64u) + 7u] = 0.5f; // band 1, bin 7
            energy_b[(3u * 64u) + 63u] = 2.0f; // band 3, last bin
            arrivals[0] = TglArrival{.onset_seconds = 0.007f, .radial_velocity = -0.3f, .energy = {0.0f, 0.5f, 0.0f, 0.0f}};
            arrivals[1] = TglArrival{.onset_seconds = 0.063f, .radial_velocity = 0.1f, .energy = {0.0f, 0.0f, 0.0f, 2.0f}};
            eyes[0] = TglEyeView{.samples = head_sample.data(), .sample_count = 1u, .channels = 1u};
            eyes[1] = TglEyeView{.samples = tail_sample.data(), .sample_count = 1u, .channels = 1u};
            ears[0] = TglEarView{.energy = energy_a.data(), .arrivals = arrivals.data(), .arrival_count = 1u, .band_count = 4u, .bin_count = 64u, .reserved0 = 0u};
            ears[1] = TglEarView{.energy = energy_b.data(), .arrivals = arrivals.data(), .arrival_count = 2u, .band_count = 4u, .bin_count = 64u, .reserved0 = 0u};
            contacts[0] = TglContact{
                .position = {0.1f, -0.2f, 0.0f}, .impulse = {0.0f, 0.4f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}, .depth = 0.0f, .slip = {0.0f, 0.0f, 0.0f}};
            contacts[1] = TglContact{
                .position = {-0.1f, -0.2f, 0.05f}, .impulse = {0.0f, 0.3f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}, .depth = 0.001f, .slip = {0.2f, 0.0f, 0.0f}};
            senses.tick = tick;
            senses.eyes = eyes.data();
            senses.ears = ears.data();
            senses.contacts = contacts.data();
            senses.eye_count = 2u;
            senses.ear_count = 2u;
            senses.contact_count = 2u;
            senses.dt_seconds = 0.03125f;
            senses.body_forward_speed = 0.5f;
            senses.specific_force[1] = 9.81f;
            senses.irradiance = 0.125f;
        }
    };

} // namespace

TEST_CASE(a_body_and_a_tick_are_copied_whole_and_nothing_that_did_not_fit_goes_uncounted)
{
    const FirstBody first;
    const WormLib::BodySnapshot body{WormLib::snapshotBody(first.desc, 0.03125f)};
    TEST_CHECK_EQUAL(body.creature_id, 256u);
    TEST_CHECK_EQUAL(body.eye_count, 2u);
    TEST_CHECK_EQUAL(body.ear_count, 2u);
    TEST_CHECK_EQUAL(body.eyes_dropped, 0u);
    TEST_CHECK_EQUAL(body.eyes[0].directions[2], -1.0f);
    TEST_CHECK_EQUAL(body.eyes[1].position[2], 0.2f);
    TEST_CHECK_EQUAL(body.ears[1].band_edges_hz[4], 13500.0f);
    TEST_CHECK_EQUAL(body.ears[0].bin_count, 64u);
    TEST_CHECK_EQUAL(body.max_turn_rate, 1.5707964f);
    TEST_CHECK_EQUAL(body.nominal_dt_seconds, 0.03125f);

    const FirstSenses tick{1000u};
    WormLib::SensesSnapshot snapshot{};
    WormLib::snapshotSenses(tick.senses, snapshot);
    TEST_CHECK_EQUAL(snapshot.tick, 1000u);
    TEST_CHECK_EQUAL(snapshot.eyes[0].samples[0], 0.75f);
    TEST_CHECK_EQUAL(snapshot.eyes[1].samples[0], 0.25f);
    // Band-major on both sides, the seam's stride on this one.
    TEST_CHECK_EQUAL(snapshot.ears[0].energy[(1u * WormLib::SEAM_EAR_BINS_MAX) + 7u], 0.5f);
    TEST_CHECK_EQUAL(snapshot.ears[1].energy[(3u * WormLib::SEAM_EAR_BINS_MAX) + 63u], 2.0f);
    TEST_CHECK_EQUAL(snapshot.ears[1].arrival_count, 2u);
    TEST_CHECK_EQUAL(snapshot.ears[1].arrivals[1].onset_seconds, 0.063f);
    TEST_CHECK_EQUAL(snapshot.contact_count, 2u);
    TEST_CHECK_EQUAL(snapshot.contacts[1].slip[0], 0.2f);
    TEST_CHECK_EQUAL(snapshot.contacts_dropped, 0u);
    TEST_CHECK_EQUAL(snapshot.specific_force[1], 9.81f);

    // A body richer than the seam: nothing is silently trimmed, every drop is counted.
    std::vector<float> many_directions(3u * 300u, 0.0f);
    std::vector<float> many_acceptance(300u, 0.1f);
    std::array<TglEyeDesc, 6> many_eyes{};
    for (TglEyeDesc& eye : many_eyes) {
        eye = TglEyeDesc{.sample_directions = many_directions.data(),
            .sample_acceptance_angles = many_acceptance.data(),
            .position = {0.0f, 0.0f, 0.0f},
            .sample_count = 300u,
            .channels = 3u,
            .quantisation_bits = 4u};
    }
    TglCreatureDesc rich{first.desc};
    rich.eyes = many_eyes.data();
    rich.eye_count = 6u;
    const WormLib::BodySnapshot rich_body{WormLib::snapshotBody(rich, 0.03125f)};
    TEST_CHECK_EQUAL(rich_body.eye_count, WormLib::SEAM_EYES_MAX);
    TEST_CHECK_EQUAL(rich_body.eyes_dropped, 6u - WormLib::SEAM_EYES_MAX);
    TEST_CHECK_EQUAL(rich_body.eyes[0].sample_count, WormLib::SEAM_EYE_SAMPLES_MAX);
    TEST_CHECK_EQUAL(rich_body.eyes[0].samples_dropped, 300u - WormLib::SEAM_EYE_SAMPLES_MAX);

    std::array<TglContact, 40> many_contacts{};
    TglSenses crowded{tick.senses};
    crowded.contacts = many_contacts.data();
    crowded.contact_count = 40u;
    WormLib::snapshotSenses(crowded, snapshot);
    TEST_CHECK_EQUAL(snapshot.contact_count, WormLib::SEAM_CONTACTS_MAX);
    TEST_CHECK_EQUAL(snapshot.contacts_dropped, 40u - WormLib::SEAM_CONTACTS_MAX);
}

TEST_CASE(the_mailbox_hands_over_the_latest_once_and_latches_a_call_until_a_tick_hears_it)
{
    WormLib::Mailbox mailbox;
    WormLib::SensesSnapshot out{};
    std::uint64_t seen{0u};
    TEST_CHECK(!mailbox.takeSenses(out, seen));
    TEST_CHECK_EQUAL(mailbox.sensesGeneration(), 0u);

    WormLib::SensesSnapshot senses{};
    senses.tick = 1u;
    mailbox.publishSenses(senses);
    senses.tick = 2u;
    mailbox.publishSenses(senses);
    TEST_CHECK(mailbox.takeSenses(out, seen));
    TEST_CHECK_EQUAL(out.tick, 2u); // latest wins; tick 1 was never seen and never will be
    TEST_CHECK_EQUAL(out.generation, 2u);
    TEST_CHECK_EQUAL(seen, 2u);
    TEST_CHECK(!mailbox.takeSenses(out, seen)); // nothing newer

    TEST_CHECK(!mailbox.takeIntent().has_value());
    mailbox.offerIntent(WormLib::Intent{.forward_speed = 0.5f, .turn_rate = 0.0f, .vocalisation = 0.0f, .generation = 0u});
    mailbox.offerIntent(WormLib::Intent{.forward_speed = 0.7f, .turn_rate = 0.1f, .vocalisation = 0.0f, .generation = 0u});
    const std::optional<WormLib::Intent> first{mailbox.takeIntent()};
    TEST_CHECK(first.has_value());
    TEST_CHECK_EQUAL(first->forward_speed, 0.7f); // latest wins
    TEST_CHECK_EQUAL(first->generation, 2u);
    TEST_CHECK(!mailbox.takeIntent().has_value()); // taken once

    // A call offered, then overwritten by the panel's next poll before any tick took: latched.
    mailbox.offerIntent(WormLib::Intent{.forward_speed = 0.7f, .turn_rate = 0.0f, .vocalisation = 0.8f, .generation = 0u});
    mailbox.offerIntent(WormLib::Intent{.forward_speed = 0.7f, .turn_rate = 0.0f, .vocalisation = 0.0f, .generation = 0u});
    const std::optional<WormLib::Intent> heard{mailbox.takeIntent()};
    TEST_CHECK(heard.has_value());
    TEST_CHECK_EQUAL(heard->vocalisation, 0.8f);
    // And delivered exactly once: the next offer without a call carries none.
    mailbox.offerIntent(WormLib::Intent{.forward_speed = 0.7f, .turn_rate = 0.0f, .vocalisation = 0.0f, .generation = 0u});
    const std::optional<WormLib::Intent> after{mailbox.takeIntent()};
    TEST_CHECK(after.has_value());
    TEST_CHECK_EQUAL(after->vocalisation, 0.0f);
}

TEST_CASE(the_worm_applies_a_fresh_intent_repeats_it_for_the_budget_and_then_brakes)
{
    const FirstBody first;
    WormLib::Worm worm{first.desc, 0.03125f};
    TEST_CHECK_EQUAL(worm.eyeCount(), 2u);
    TEST_CHECK_EQUAL(worm.body().max_forward_speed, 1.0f);

    // Nobody steering: the worm stands, and says so.
    FirstSenses tick{1u};
    TglActions actions{};
    worm.tick(tick.senses, actions);
    TEST_CHECK(worm.lastApplied() == WormLib::Applied::Braked);
    TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.0f);
    // What it sensed was published for a panel to take.
    WormLib::SensesSnapshot seen_senses{};
    std::uint64_t seen{0u};
    TEST_CHECK(worm.mailbox().takeSenses(seen_senses, seen));
    TEST_CHECK_EQUAL(seen_senses.tick, 1u);
    TEST_CHECK_EQUAL(seen_senses.eyes[0].samples[0], 0.75f);

    // The panel speaks: applied fresh, with the call.
    worm.mailbox().offerIntent(WormLib::Intent{.forward_speed = 0.6f, .turn_rate = -0.2f, .vocalisation = 1.0f, .generation = 0u});
    tick.senses.tick = 2u;
    actions = TglActions{};
    worm.tick(tick.senses, actions);
    TEST_CHECK(worm.lastApplied() == WormLib::Applied::Fresh);
    TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.6f);
    TEST_CHECK_EQUAL(actions.desired_turn_rate, -0.2f);
    TEST_CHECK_EQUAL(actions.vocalisation_strength, 1.0f);
    TEST_CHECK_EQUAL(worm.repeatBudget(), WormLib::PANEL_REPEAT_TICKS);

    // Silence: the motion repeats for the budget, the voice does not.
    for (std::uint32_t repeat{1u}; repeat <= WormLib::PANEL_REPEAT_TICKS; ++repeat) {
        tick.senses.tick = 2u + repeat;
        actions = TglActions{};
        worm.tick(tick.senses, actions);
        TEST_CHECK(worm.lastApplied() == WormLib::Applied::Repeated);
        TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.6f);
        TEST_CHECK_EQUAL(actions.desired_turn_rate, -0.2f);
        TEST_CHECK_EQUAL(actions.vocalisation_strength, 0.0f);
        TEST_CHECK_EQUAL(worm.repeatBudget(), WormLib::PANEL_REPEAT_TICKS - repeat);
    }
    // Past the budget: the brake.
    tick.senses.tick = 100u;
    actions = TglActions{};
    worm.tick(tick.senses, actions);
    TEST_CHECK(worm.lastApplied() == WormLib::Applied::Braked);
    TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.0f);
    TEST_CHECK_EQUAL(actions.desired_turn_rate, 0.0f);
    TEST_CHECK_EQUAL(worm.ticksSeen(), 2u + WormLib::PANEL_REPEAT_TICKS + 1u);

    // A word breaks the silence and the budget refills.
    worm.mailbox().offerIntent(WormLib::Intent{.forward_speed = -0.3f, .turn_rate = 0.0f, .vocalisation = 0.0f, .generation = 0u});
    tick.senses.tick = 101u;
    actions = TglActions{};
    worm.tick(tick.senses, actions);
    TEST_CHECK(worm.lastApplied() == WormLib::Applied::Fresh);
    TEST_CHECK_EQUAL(actions.desired_forward_speed, -0.3f);
    TEST_CHECK_EQUAL(worm.repeatBudget(), WormLib::PANEL_REPEAT_TICKS);
}

TEST_CASE(a_panel_on_another_thread_never_loses_the_newest_word_and_the_tick_never_waits_for_it)
{
    const FirstBody first;
    WormLib::Worm worm{first.desc, 0.03125f};
    constexpr std::uint32_t OFFERS{2000u};
    // The panel: offers a rising speed as fast as it can, taking senses as it goes.
    std::thread panel{[&worm] {
        WormLib::SensesSnapshot senses{};
        std::uint64_t seen{0u};
        for (std::uint32_t offer{1u}; offer <= OFFERS; ++offer) {
            worm.mailbox().offerIntent(WormLib::Intent{.forward_speed = static_cast<float>(offer) * 0.0001f, .turn_rate = 0.0f, .vocalisation = 0.0f, .generation = 0u});
            (void)worm.mailbox().takeSenses(senses, seen);
        }
    }};
    // The tick, meanwhile: every tick's answer is one of the offers, never older than the last.
    FirstSenses tick{1u};
    float last{0.0f};
    for (std::uint64_t step{1u}; step <= 400u; ++step) {
        tick.senses.tick = step;
        TglActions actions{};
        worm.tick(tick.senses, actions);
        if (worm.lastApplied() == WormLib::Applied::Fresh) {
            TEST_CHECK(actions.desired_forward_speed >= last);
            last = actions.desired_forward_speed;
        }
    }
    panel.join();
    // After the panel's last word, the tick hears exactly it.
    tick.senses.tick = 1000u;
    TglActions actions{};
    worm.tick(tick.senses, actions);
    const std::optional<WormLib::Intent> none{worm.mailbox().takeIntent()};
    TEST_CHECK(!none.has_value());
    TEST_CHECK_EQUAL(worm.mailbox().intentGeneration(), static_cast<std::uint64_t>(OFFERS));
    TEST_CHECK(worm.mailbox().sensesGeneration() >= 401u);
}

int main()
{
    return static_cast<int>(TestingLib::runAll());
}
