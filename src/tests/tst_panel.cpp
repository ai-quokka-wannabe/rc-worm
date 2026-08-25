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
    The window, driven on the test's own Qt thread: it draws what the mailbox holds, offers what
    the keys say, latches a call, and never makes the publisher wait. Shapes adopted from the
    owner's claude-chats-browser: a tight bound on the thing that must be fast, a generous
    QTRY_ on the thing that must merely happen, the measured value in every failure message.
*/

#include "../panel/window.hpp"

#include <seam.hpp>

#include <QtCore/QElapsedTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtWidgets/QApplication>

#include <atomic>
#include <cstdint>
#include <thread>

namespace
{

    WormLib::BodySnapshot firstBody()
    {
        WormLib::BodySnapshot body{};
        body.creature_id = 256u;
        body.eye_count = 2u;
        body.ear_count = 2u;
        body.max_contact_count = 4u;
        body.max_forward_speed = 1.0f;
        body.max_turn_rate = 1.5707964f;
        body.max_vocalisation_strength = 1.0f;
        body.nominal_dt_seconds = 0.03125f;
        for (std::uint32_t index{0u}; index < 2u; ++index) {
            WormLib::EyeStation& eye{body.eyes[index]};
            eye.position[2] = index == 0u ? -0.2f : 0.2f;
            eye.directions[2] = index == 0u ? -1.0f : 1.0f;
            eye.acceptance_radians[0] = 0.5235988f;
            eye.sample_count = 1u;
            eye.channels = 1u;
            WormLib::EarStation& ear{body.ears[index]};
            ear.position[2] = index == 0u ? -0.2f : 0.2f;
            const float edges[5]{2000.0f, 4500.0f, 7500.0f, 10500.0f, 13500.0f};
            for (std::uint32_t edge{0u}; edge < 5u; ++edge) {
                ear.band_edges_hz[edge] = edges[edge];
            }
            ear.band_count = 4u;
            ear.bin_count = 64u;
            ear.bin_seconds = 0.001f;
        }
        return body;
    }

    WormLib::SensesSnapshot aTick(const std::uint64_t tick)
    {
        WormLib::SensesSnapshot senses{};
        senses.tick = tick;
        senses.eye_count = 2u;
        senses.ear_count = 2u;
        senses.contact_count = 1u;
        senses.dt_seconds = 0.03125f;
        senses.eyes[0].sample_count = 1u;
        senses.eyes[0].channels = 1u;
        senses.eyes[0].samples[0] = 0.75f;
        senses.eyes[1].sample_count = 1u;
        senses.eyes[1].channels = 1u;
        senses.ears[0].band_count = 4u;
        senses.ears[0].bin_count = 64u;
        senses.ears[0].energy[(1u * WormLib::SEAM_EAR_BINS_MAX) + 7u] = 0.5f;
        senses.ears[0].arrival_count = 1u;
        senses.ears[0].arrivals[0] = TglArrival{.onset_seconds = 0.007f, .radial_velocity = -0.3f, .energy = {0.0f, 0.5f, 0.0f, 0.0f}};
        senses.ears[1].band_count = 4u;
        senses.ears[1].bin_count = 64u;
        senses.contacts[0] = TglContact{
            .position = {0.1f, -0.2f, 0.0f}, .impulse = {0.0f, 0.4f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}, .depth = 0.0f, .slip = {0.0f, 0.0f, 0.0f}};
        senses.specific_force[1] = 9.81f;
        return senses;
    }

} // namespace

class PanelTests : public QObject {
    Q_OBJECT

private slots:
    void the_window_draws_the_newest_senses_and_offers_what_the_keys_say()
    {
        WormLib::Mailbox mailbox;
        const WormLib::BodySnapshot body{firstBody()};
        PanelLib::PanelWindow window{mailbox, body};
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY2(window.statusText().contains(QStringLiteral("waiting for the first tick")), qPrintable(window.statusText()));

        // A tick published from the worm's side: the window takes it on its next poll.
        mailbox.publishSenses(aTick(77u));
        QTRY_COMPARE_WITH_TIMEOUT(window.sensesSeen(), 1u, 5000);
        QVERIFY2(window.statusText().contains(QStringLiteral("tick 77")), qPrintable(window.statusText()));

        // Every poll offers: a silent panel is a panel that stopped polling, not one that said nothing.
        QTRY_VERIFY_WITH_TIMEOUT(mailbox.intentGeneration() >= 3u, 5000);
        std::optional<WormLib::Intent> intent{mailbox.takeIntent()};
        QVERIFY(intent.has_value());
        QCOMPARE(intent->forward_speed, 0.0f);
        QCOMPARE(intent->turn_rate, 0.0f);

        // W held: full forward, at the body's bound. A held: full turn to the left, positive.
        QTest::keyPress(&window, Qt::Key_W);
        QTest::keyPress(&window, Qt::Key_A);
        QCOMPARE(window.currentIntent().forward_speed, body.max_forward_speed);
        QCOMPARE(window.currentIntent().turn_rate, body.max_turn_rate);
        // A take consumes, and QTRY_ evaluates once more after it is satisfied: what was heard
        // sticks, so the re-evaluation reads the same answer.
        WormLib::Intent heard{};
        QTRY_VERIFY_WITH_TIMEOUT(
            [&] {
                if (const std::optional<WormLib::Intent> taken{mailbox.takeIntent()}) {
                    heard = *taken;
                }
                return (heard.forward_speed == body.max_forward_speed) && (heard.turn_rate == body.max_turn_rate);
            }(),
            5000);

        // Released: back to the sliders, which sit at zero - the brake.
        QTest::keyRelease(&window, Qt::Key_W);
        QTest::keyRelease(&window, Qt::Key_A);
        QCOMPARE(window.currentIntent().forward_speed, 0.0f);
        QCOMPARE(window.currentIntent().turn_rate, 0.0f);

        // Space: one call, at the voice slider's strength, latched in the mailbox until taken.
        QTest::keyPress(&window, Qt::Key_Space);
        QCOMPARE(window.currentIntent().vocalisation, body.max_vocalisation_strength);
        WormLib::Intent called{};
        QTRY_VERIFY_WITH_TIMEOUT(
            [&] {
                if (const std::optional<WormLib::Intent> taken{mailbox.takeIntent()}) {
                    called = *taken;
                }
                return called.vocalisation == body.max_vocalisation_strength;
            }(),
            5000);
        // And once only: the polls after it carry no voice.
        QTest::qWait(100);
        const std::optional<WormLib::Intent> quiet{mailbox.takeIntent()};
        QVERIFY(quiet.has_value());
        QCOMPARE(quiet->vocalisation, 0.0f);

        window.stopPolling();
    }

    void publishing_never_waits_for_the_window()
    {
        WormLib::Mailbox mailbox;
        const WormLib::BodySnapshot body{firstBody()};
        PanelLib::PanelWindow window{mailbox, body};
        window.setPollIntervalMs(5);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        // The tick's side, on its own thread: two thousand publishes while the window draws.
        std::atomic<qint64> slowest_ns{0};
        std::atomic<bool> done{false};
        std::thread tick{[&] {
            QElapsedTimer stopwatch;
            for (std::uint64_t step{1u}; step <= 2000u; ++step) {
                const WormLib::SensesSnapshot senses{aTick(step)};
                stopwatch.start();
                mailbox.publishSenses(senses);
                const qint64 took{stopwatch.nsecsElapsed()};
                if (took > slowest_ns.load()) {
                    slowest_ns.store(took);
                }
            }
            done.store(true);
        }};
        QTRY_VERIFY_WITH_TIMEOUT(done.load(), 30000);
        tick.join();
        // A publish is one copy under a mutex the window holds only for a copy of its own: a
        // generous bound, because this is a shared runner, and the measured value on failure.
        const double slowest_ms{static_cast<double>(slowest_ns.load()) / 1.0e6};
        QVERIFY2(slowest_ms < 50.0, qPrintable(QStringLiteral("the slowest publish took %1 ms").arg(slowest_ms)));
        QTRY_COMPARE_WITH_TIMEOUT(window.sensesSeen(), 2000u, 5000);
        QVERIFY2(window.statusText().contains(QStringLiteral("tick 2000")), qPrintable(window.statusText()));
        window.stopPolling();
    }
};

QTEST_MAIN(PanelTests)

#include "tst_panel.moc"
