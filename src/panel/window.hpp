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

#include <seam.hpp>

#include <QtCore/QString>
#include <QtWidgets/QWidget>

#include <cstdint>

class QLabel;
class QSlider;
class QTimer;

/*!
    The window: the senses as the worm has them, and the three controls the ABI carries. Lives on
    the Qt thread, polls the mailbox on a timer (never a signal from the tick - the tick knows no
    window), converts the seam's plain structs at its own edge, and offers an intent every poll so
    the worm's silence rule sees a live panel as a stream and a dead one as silence.
*/
namespace PanelLib
{

    //! One eye: its samples drawn where they look, on an equirectangular map of the body frame.
    class EyeView : public QWidget {
        Q_OBJECT

    public:
        EyeView(const WormLib::EyeStation& station, std::uint32_t index, QWidget* parent = nullptr);

        void showView(const WormLib::EyeSnapshot& view);

        [[nodiscard]] QSize sizeHint() const override;

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        WormLib::EyeStation m_station;
        WormLib::EyeSnapshot m_view{};
        std::uint32_t m_index;
    };

    //! One ear: the band-by-bin histogram it is, with the arrivals' onsets and radial velocities marked.
    class EarView : public QWidget {
        Q_OBJECT

    public:
        EarView(const WormLib::EarStation& station, std::uint32_t index, QWidget* parent = nullptr);

        void showView(const WormLib::EarSnapshot& view, std::uint64_t tick);

        [[nodiscard]] QSize sizeHint() const override;

        //! The tick the last arrivals came in, zero if none ever did - for a test, and the status.
        [[nodiscard]] std::uint64_t lastArrivalTick() const noexcept
        {
            return m_last_arrival_tick;
        }

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        WormLib::EarStation m_station;
        WormLib::EarSnapshot m_view{};
        //! An arrival is one tick's event, gone before an eye can see it: the last ones are kept
        //! and drawn dimmed for a second, with the tick they came in.
        TglArrival m_last_arrivals[TGL_EAR_ARRIVALS_MAX]{};
        std::uint32_t m_last_arrival_count{0u};
        std::uint64_t m_last_arrival_tick{0u};
        std::uint64_t m_tick{0u};
        std::uint32_t m_index;
    };

    //! The feel: every contact where it happened on the body, the vestibular numbers beside.
    class FeelView : public QWidget {
        Q_OBJECT

    public:
        explicit FeelView(QWidget* parent = nullptr);

        void showSenses(const WormLib::SensesSnapshot& senses);

        [[nodiscard]] QSize sizeHint() const override;

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        WormLib::SensesSnapshot m_senses{};
        bool m_have_senses{false};
    };

    class PanelWindow : public QWidget {
        Q_OBJECT

    public:
        PanelWindow(WormLib::Mailbox& mailbox, const WormLib::BodySnapshot& body, QWidget* parent = nullptr);
        ~PanelWindow() override;

        PanelWindow(const PanelWindow&) = delete;
        PanelWindow& operator=(const PanelWindow&) = delete;

        //! Stops the poll timer so the mailbox is never read again; called before the window dies.
        void stopPolling();

        //! What the status line says: tick, generations, silence - for a test to read.
        [[nodiscard]] QString statusText() const;

        //! The generation of the newest senses this window has drawn.
        [[nodiscard]] std::uint64_t sensesSeen() const noexcept
        {
            return m_seen;
        }

        //! The intent this window would offer now, from its sliders and held keys.
        [[nodiscard]] WormLib::Intent currentIntent() const noexcept;

        //! How often the mailbox is polled; the default is a little faster than the Grid ticks.
        void setPollIntervalMs(int milliseconds);

    protected:
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    private:
        void poll();
        void brake();
        void updateHeader();

        WormLib::Mailbox& m_mailbox;
        WormLib::BodySnapshot m_body;
        WormLib::SensesSnapshot m_senses{};
        std::uint64_t m_seen{0u};
        std::uint32_t m_silent_polls{0u};
        std::uint64_t m_offered{0u};

        bool m_forward_held{false};
        bool m_reverse_held{false};
        bool m_left_held{false};
        bool m_right_held{false};
        bool m_call_pending{false};

        QTimer* m_poll{nullptr};
        QLabel* m_header{nullptr};
        QLabel* m_status{nullptr};
        QLabel* m_intent_label{nullptr};
        QSlider* m_forward{nullptr};
        QSlider* m_turn{nullptr};
        QSlider* m_voice{nullptr};
        FeelView* m_feel{nullptr};
        QList<EyeView*> m_eyes;
        QList<EarView*> m_ears;
    };

} // namespace PanelLib
