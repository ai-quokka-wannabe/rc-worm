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

#include "window.hpp"

#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QScreen>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace PanelLib
{

    namespace
    {

        // The Grid's own palette: black glass, cyan for the world, orange for its accents, green for
        // the worm, magenta for what is wrong.
        const QColor GLASS{7, 9, 13};
        const QColor INK{214, 245, 255};
        const QColor DIM{120, 140, 150};
        const QColor CYAN{34, 211, 238};
        const QColor ORANGE{249, 115, 22};
        const QColor GREEN{74, 222, 128};
        const QColor MAGENTA{232, 121, 249};

        //! Linear radiance to a display level: the same shape the Grid's own tone map has, no clamp before.
        float tone(const float linear) noexcept
        {
            if (!(linear > 0.0f)) {
                return 0.0f;
            }
            return 1.0f - std::exp(-linear);
        }

        int level(const float linear) noexcept
        {
            return std::clamp(static_cast<int>(std::lround(tone(linear) * 255.0f)), 0, 255);
        }

        QString vec3(const float (&v)[3])
        {
            return QStringLiteral("(%1, %2, %3)").arg(v[0], 0, 'f', 2).arg(v[1], 0, 'f', 2).arg(v[2], 0, 'f', 2);
        }

        float length3(const float (&v)[3]) noexcept
        {
            return std::sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
        }

        //! The slider's integer as a fraction of the body's bound, in [-1, 1] or [0, 1].
        float fraction(const QSlider& slider) noexcept
        {
            return static_cast<float>(slider.value()) / 100.0f;
        }

    } // namespace

    // ---- EyeView --------------------------------------------------------------------------------

    EyeView::EyeView(const WormLib::EyeStation& station, const std::uint32_t index, QWidget* parent) :
        QWidget(parent),
        m_station(station),
        m_index(index)
    {
        setMinimumSize(260, 130);
    }

    void EyeView::showView(const WormLib::EyeSnapshot& view)
    {
        m_view = view;
        update();
    }

    QSize EyeView::sizeHint() const
    {
        return {260, 130};
    }

    void EyeView::paintEvent(QPaintEvent* const event)
    {
        (void)event;
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), GLASS);
        const QRectF frame{8.0, 22.0, width() - 16.0, height() - 30.0};
        painter.setPen(QPen{DIM, 1.0});
        painter.drawRect(frame);
        // The map sits inside the frame by the largest disc's radius, so a sample looking straight
        // back - azimuth pi, the map's edge - is drawn whole rather than cut.
        double largest{4.0};
        for (std::uint32_t sample{0u}; sample < m_station.sample_count; ++sample) {
            largest = std::max(largest, (m_station.acceptance_radians[sample] / (2.0 * std::numbers::pi)) * frame.width());
        }
        const QRectF map{frame.adjusted(largest, largest, -largest, -largest)};
        // The equirectangular map of the body frame: forward (-Z) at the centre, up at the top.
        painter.drawLine(QPointF{map.center().x(), map.top()}, QPointF{map.center().x(), map.bottom()});
        painter.drawLine(QPointF{map.left(), map.center().y()}, QPointF{map.right(), map.center().y()});

        for (std::uint32_t sample{0u}; sample < m_station.sample_count; ++sample) {
            const float* const direction{&m_station.directions[sample * 3u]};
            const float azimuth{std::atan2(direction[0], -direction[2])};
            const float elevation{std::asin(std::clamp(direction[1], -1.0f, 1.0f))};
            const double x{map.center().x() + ((azimuth / std::numbers::pi_v<float>)*(map.width() / 2.0))};
            const double y{map.center().y() - ((elevation / (std::numbers::pi_v<float> / 2.0f)) * (map.height() / 2.0))};
            const double radius{std::max(4.0, (m_station.acceptance_radians[sample] / (2.0 * std::numbers::pi)) * map.width())};

            QColor fill{GLASS};
            if ((sample < m_view.sample_count) && (m_view.channels > 0u)) {
                const float* const values{&m_view.samples[sample * m_view.channels]};
                if (m_view.channels >= 3u) {
                    fill = QColor{level(values[0]), level(values[1]), level(values[2])};
                } else {
                    const int grey{level(values[0])};
                    fill = QColor{grey, grey, grey};
                }
            }
            painter.setBrush(fill);
            painter.setPen(QPen{GREEN, 1.0});
            painter.drawEllipse(QPointF{x, y}, radius, radius);
        }

        painter.setPen(INK);
        QString title{QStringLiteral("eye %1  at %2 m  %3 sample(s) x %4  %5 bit")
                .arg(m_index)
                .arg(vec3(m_station.position))
                .arg(m_station.sample_count)
                .arg(m_station.channels)
                .arg(m_station.quantisation_bits == 0u ? QStringLiteral("un-quantised") : QString::number(m_station.quantisation_bits))};
        if ((m_station.sample_count == 1u) && (m_view.sample_count == 1u)) {
            title += QStringLiteral("   value %1").arg(m_view.samples[0], 0, 'f', 3);
        }
        painter.drawText(QPointF{8.0, 15.0}, title);
        if ((m_station.samples_dropped > 0u) || (m_view.samples_dropped > 0u)) {
            painter.setPen(MAGENTA);
            painter.drawText(QPointF{map.left() + 4.0, map.bottom() - 4.0},
                QStringLiteral("%1 sample(s) beyond the seam's capacity, not shown").arg(std::max(m_station.samples_dropped, m_view.samples_dropped)));
        }
    }

    // ---- EarView --------------------------------------------------------------------------------

    EarView::EarView(const WormLib::EarStation& station, const std::uint32_t index, QWidget* parent) :
        QWidget(parent),
        m_station(station),
        m_index(index)
    {
        setMinimumSize(360, 130);
    }

    void EarView::showView(const WormLib::EarSnapshot& view, const std::uint64_t tick)
    {
        m_view = view;
        m_tick = tick;
        if (view.arrival_count > 0u) {
            m_last_arrival_count = std::min(view.arrival_count, TGL_EAR_ARRIVALS_MAX);
            for (std::uint32_t index{0u}; index < m_last_arrival_count; ++index) {
                m_last_arrivals[index] = view.arrivals[index];
            }
            m_last_arrival_tick = tick;
        }
        update();
    }

    QSize EarView::sizeHint() const
    {
        return {340, 130};
    }

    void EarView::paintEvent(QPaintEvent* const event)
    {
        (void)event;
        QPainter painter{this};
        painter.fillRect(rect(), GLASS);
        const double label_width{78.0};
        const QRectF grid{8.0 + label_width, 22.0, width() - 16.0 - label_width, height() - 30.0};
        const std::uint32_t bands{std::max(1u, m_station.band_count)};
        const std::uint32_t bins{std::max(1u, m_station.bin_count)};
        const double cell_w{grid.width() / bins};
        const double cell_h{grid.height() / bands};

        for (std::uint32_t band{0u}; band < m_station.band_count; ++band) {
            for (std::uint32_t bin{0u}; bin < m_station.bin_count; ++bin) {
                float energy{0.0f};
                if ((band < m_view.band_count) && (bin < m_view.bin_count)) {
                    energy = m_view.energy[(band * WormLib::SEAM_EAR_BINS_MAX) + bin];
                }
                const int lit{level(energy)};
                const QColor cell{(CYAN.red() * lit) / 255, (CYAN.green() * lit) / 255, (CYAN.blue() * lit) / 255};
                painter.fillRect(QRectF{grid.left() + (bin * cell_w), grid.top() + (band * cell_h), std::ceil(cell_w), std::ceil(cell_h)}, cell);
            }
            painter.setPen(DIM);
            painter.drawText(QRectF{8.0, grid.top() + (band * cell_h), label_width - 4.0, cell_h}, Qt::AlignRight | Qt::AlignVCenter,
                QStringLiteral("%1-%2 kHz").arg(m_station.band_edges_hz[band] / 1000.0f, 0, 'f', 1).arg(m_station.band_edges_hz[band + 1u] / 1000.0f, 0, 'f', 1));
        }
        painter.setPen(QPen{DIM, 1.0});
        painter.drawRect(grid);

        // The arrivals: onset as a column, radial velocity as a colour - approaching cyan, receding
        // orange. This tick's at full strength; the last ones for a second after, dimmed, because
        // an arrival is one tick's event and a human reads slower than the Grid ticks.
        const bool remembered{(m_last_arrival_count > 0u) && (m_tick >= m_last_arrival_tick) && (m_tick - m_last_arrival_tick < 32u)};
        const bool fresh{m_view.arrival_count > 0u};
        const TglArrival* const shown{fresh ? m_view.arrivals : m_last_arrivals};
        const std::uint32_t shown_count{fresh ? m_view.arrival_count : (remembered ? m_last_arrival_count : 0u)};
        for (std::uint32_t arrival{0u}; arrival < shown_count; ++arrival) {
            const TglArrival& a{shown[arrival]};
            if (!(m_station.bin_seconds > 0.0f)) {
                break;
            }
            const double column{a.onset_seconds / m_station.bin_seconds};
            if ((column < 0.0) || (column > bins)) {
                continue;
            }
            const double x{grid.left() + (column * cell_w)};
            QColor tint{a.radial_velocity < 0.0f ? CYAN : ORANGE};
            if (!fresh) {
                tint.setAlpha(110);
            }
            painter.setPen(QPen{tint, 2.0});
            painter.drawLine(QPointF{x, grid.top() - 4.0}, QPointF{x, grid.bottom() + 4.0});
        }
        if (m_last_arrival_tick > 0u) {
            painter.setPen(DIM);
            const TglArrival& a{m_last_arrivals[0]};
            painter.drawText(QPointF{grid.left() + 4.0, grid.bottom() - 4.0},
                QStringLiteral("last arrival(s): tick %1, %2 of them, first at %3 ms, %4 m/s")
                    .arg(m_last_arrival_tick)
                    .arg(m_last_arrival_count)
                    .arg(a.onset_seconds * 1000.0f, 0, 'f', 2)
                    .arg(a.radial_velocity, 0, 'f', 2));
        }

        painter.setPen(INK);
        painter.drawText(QPointF{8.0, 15.0},
            QStringLiteral("ear %1  at %2 m  %3 bands x %4 bins of %5 ms  %6 arrival(s)")
                .arg(m_index)
                .arg(vec3(m_station.position))
                .arg(m_station.band_count)
                .arg(m_station.bin_count)
                .arg(m_station.bin_seconds * 1000.0f, 0, 'f', 1)
                .arg(m_view.arrival_count));
        if ((m_station.bands_dropped > 0u) || (m_station.bins_dropped > 0u)) {
            painter.setPen(MAGENTA);
            painter.drawText(QPointF{grid.left() + 4.0, grid.bottom() - 4.0},
                QStringLiteral("%1 band(s), %2 bin(s) beyond the seam's capacity, not shown").arg(m_station.bands_dropped).arg(m_station.bins_dropped));
        }
    }

    // ---- FeelView -------------------------------------------------------------------------------

    FeelView::FeelView(const WormLib::BodySnapshot& body, QWidget* parent) :
        QWidget(parent),
        m_segment_count(body.segment_count),
        m_segment_spacing(body.segment_spacing),
        m_segment_radius(body.segment_radius)
    {
        // Low enough to squeeze onto a narrow screen: the rows elide at the plan's edge and
        // the chain scales to the width it gets, so nothing is lost when the layout presses.
        setMinimumSize(340, 260);
    }

    void FeelView::showSenses(const WormLib::SensesSnapshot& senses)
    {
        m_senses = senses;
        m_have_senses = true;
        update();
    }

    QSize FeelView::sizeHint() const
    {
        // At least the minimum: a hint below it made the layout plan a column the minimum then
        // pushed past the window's edge, and the feel was born clipped.
        return {560, 260};
    }

    double FeelView::drawChain(QPainter& painter) const
    {
        if ((m_segment_count == 0u) || !(m_segment_spacing > 0.0f) || !(m_segment_radius > 0.0f)) {
            return height() - 4.0;
        }
        // The declaration, side on: forward to the right, the floor under the spikes, the head
        // brightest. Side on, an icosahedron's visible outline is its neon - near-black mirror
        // faces, green tubes on every edge - so the silhouettes wear the worm's green. Nothing
        // here moves: the panel knows the chain only as rez declared it; where it is and how
        // it waves is the world's, heard through the ears, never echoed back.
        const QRectF strip{8.0, height() - 64.0, width() - 16.0, 56.0};
        const double spacing{m_segment_spacing};
        const double stub{std::max(0.0, (spacing - (2.0 * m_segment_radius)) / 2.0)};
        const double nose_to_tail{(static_cast<double>(m_segment_count - 1u) * spacing) + (2.0 * (m_segment_radius + stub))};
        const double scale{std::min(strip.width() / nose_to_tail, (strip.height() - 26.0) / (2.0 * m_segment_radius))};
        const double radius{m_segment_radius * scale};
        const double floor_y{strip.bottom() - 10.0};
        const double centre_y{floor_y - radius};

        painter.setPen(QPen{DIM, 1.0});
        painter.drawLine(QPointF{strip.left(), floor_y}, QPointF{strip.right(), floor_y});
        painter.drawText(QPointF{strip.left(), strip.top() + 9.0},
            QStringLiteral("declared: a chain of %1 segment(s), %2 m apart, spike to spike").arg(m_segment_count).arg(spacing, 0, 'f', 2));

        const double head_x{strip.left() + ((strip.width() + (nose_to_tail * scale)) / 2.0) - ((m_segment_radius + stub) * scale)};
        for (std::uint32_t index{0u}; index < m_segment_count; ++index) {
            const bool head{index == 0u};
            const double x{head_x - (static_cast<double>(index) * spacing * scale)};
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen{GREEN, head ? 2.0 : 1.0});
            painter.drawEllipse(QPointF{x, centre_y}, radius, radius);
            // The two joint spikes and their neon stubs: consecutive segments' stubs meet tip
            // to tip; the nose's and the last tail's stand proud, as the body authors them.
            painter.drawLine(QPointF{x + radius, centre_y}, QPointF{x + radius + (stub * scale), centre_y});
            painter.drawLine(QPointF{x - radius, centre_y}, QPointF{x - radius - (stub * scale), centre_y});
            if (head) {
                // The eye end, said with a dot of ink just inside the nose.
                painter.setBrush(INK);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF{x + (radius * 0.6), centre_y - (radius * 0.3)}, 2.0, 2.0);
            }
        }
        return strip.top() - 4.0;
    }

    void FeelView::paintEvent(QPaintEvent* const event)
    {
        (void)event;
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), GLASS);
        painter.setPen(INK);
        painter.drawText(QPointF{8.0, 15.0}, QStringLiteral("feel"));
        const double rows_end{drawChain(painter)};
        if (!m_have_senses) {
            painter.setPen(DIM);
            painter.drawText(QPointF{8.0, 34.0}, QStringLiteral("nothing sensed yet"));
            return;
        }
        const WormLib::SensesSnapshot& s{m_senses};
        const double line{15.0};
        double y{34.0};

        // The body from above sits top right; a text row that shares its height stops at its
        // edge rather than writing straight through it (the owner's report, 2026-08-28).
        const QRectF plan{width() - 128.0, 26.0, 120.0, 120.0};
        const auto write{[this, &painter, &plan, line](const double at_y, const QString& text) {
            const bool beside_plan{at_y > (plan.top() - 4.0) && (at_y - line) < plan.bottom()};
            const int room{static_cast<int>((beside_plan ? plan.left() : width()) - 8.0 - 8.0)};
            painter.drawText(QPointF{8.0, at_y}, painter.fontMetrics().elidedText(text, Qt::ElideRight, room));
        }};

        painter.setPen(DIM);
        write(y,
            QStringLiteral("forward %1 m/s  vertical %2 m/s  turn %3 rad/s")
                .arg(s.body_forward_speed, 0, 'f', 2)
                .arg(s.body_vertical_speed, 0, 'f', 2)
                .arg(s.body_turn_rate, 0, 'f', 2));
        y += line;
        write(y, QStringLiteral("specific force %1 m/s2  |%2|").arg(vec3(s.specific_force)).arg(length3(s.specific_force), 0, 'f', 2));
        y += line;
        write(y, QStringLiteral("angular velocity %1 rad/s").arg(vec3(s.angular_velocity)));
        y += line;
        write(y, QStringLiteral("irradiance %1").arg(s.irradiance, 0, 'f', 3));
        y += line;
        painter.setPen(INK);
        write(y, QStringLiteral("%1 contact(s)").arg(s.contact_count));
        if (s.contacts_dropped > 0u) {
            painter.setPen(MAGENTA);
            painter.drawText(QPointF{110.0, y}, QStringLiteral("+ %1 beyond the seam's capacity").arg(s.contacts_dropped));
        }
        y += line;
        painter.setPen(QPen{DIM, 1.0});
        painter.drawRect(plan);
        painter.drawEllipse(plan.center(), 30.0, 30.0);
        const double scale{30.0 / 0.3};
        for (std::uint32_t index{0u}; index < s.contact_count; ++index) {
            const TglContact& c{s.contacts[index]};
            const QPointF at{plan.center().x() + (c.position[0] * scale), plan.center().y() + (c.position[2] * scale)};
            painter.setPen(QPen{GREEN, 1.5});
            painter.setBrush(GREEN);
            painter.drawEllipse(at, 3.0, 3.0);
            painter.drawLine(at, QPointF{at.x() + (c.normal[0] * 14.0), at.y() + (c.normal[2] * 14.0)});
            painter.setPen(DIM);
            const QString row{QStringLiteral("at %1  impulse %2 Ns  normal %3  depth %4  slip %5")
                    .arg(vec3(c.position))
                    .arg(length3(c.impulse), 0, 'f', 3)
                    .arg(vec3(c.normal))
                    .arg(c.depth, 0, 'f', 3)
                    .arg(length3(c.slip), 0, 'f', 2)};
            if (y < rows_end) {
                write(y, row);
                y += line;
            }
        }
    }

    // ---- PanelWindow ----------------------------------------------------------------------------

    PanelWindow::PanelWindow(WormLib::Mailbox& mailbox, const WormLib::BodySnapshot& body, QWidget* parent) :
        QWidget(parent),
        m_mailbox(mailbox),
        m_body(body)
    {
        setObjectName(QStringLiteral("rc_worm_panel"));
        setWindowTitle(QStringLiteral("rc-worm - creature %1").arg(m_body.creature_id));
        // Scoped to this window by its name: the host process owns whatever else Qt draws.
        setStyleSheet(QStringLiteral("QWidget#rc_worm_panel, QWidget#rc_worm_panel QWidget { background-color: #07090d; color: #d6f5ff; }"
                                     "QWidget#rc_worm_panel QGroupBox { border: 1px solid #1f2933; margin-top: 8px; }"
                                     "QWidget#rc_worm_panel QGroupBox::title { color: #22d3ee; subcontrol-origin: margin; left: 8px; }"
                                     "QWidget#rc_worm_panel QSlider::groove:horizontal { height: 4px; background: #1f2933; }"
                                     "QWidget#rc_worm_panel QSlider::handle:horizontal { width: 12px; margin: -6px 0; background: #4ade80; border-radius: 6px; }"
                                     "QWidget#rc_worm_panel QPushButton { border: 1px solid #22d3ee; padding: 3px 10px; }"));
        setFocusPolicy(Qt::StrongFocus);

        auto* const column{new QVBoxLayout{this}};
        m_header = new QLabel{this};
        column->addWidget(m_header);

        auto* const senses{new QHBoxLayout{}};
        auto* const eyes{new QVBoxLayout{}};
        for (std::uint32_t index{0u}; index < m_body.eye_count; ++index) {
            auto* const view{new EyeView{m_body.eyes[index], index, this}};
            m_eyes.append(view);
            eyes->addWidget(view);
        }
        eyes->addStretch(1);
        auto* const ears{new QVBoxLayout{}};
        for (std::uint32_t index{0u}; index < m_body.ear_count; ++index) {
            auto* const view{new EarView{m_body.ears[index], index, this}};
            m_ears.append(view);
            ears->addWidget(view);
        }
        ears->addStretch(1);
        m_feel = new FeelView{m_body, this};
        senses->addLayout(eyes, 2);
        senses->addLayout(ears, 3);
        senses->addWidget(m_feel, 3);
        column->addLayout(senses, 1);

        auto* const controls{new QGroupBox{QStringLiteral("controls  -  W/S forward, A/D turn, Space call, X brake; sliders hold a course"), this}};
        auto* const rows{new QVBoxLayout{controls}};
        const auto slider_row{[this, rows](const QString& name, const int low, QSlider*& slider) {
            auto* const row{new QHBoxLayout{}};
            auto* const label{new QLabel{name, this}};
            label->setMinimumWidth(150);
            slider = new QSlider{Qt::Horizontal, this};
            slider->setRange(low, 100);
            slider->setValue(0);
            slider->setFocusPolicy(Qt::NoFocus);
            row->addWidget(label);
            row->addWidget(slider, 1);
            rows->addLayout(row);
        }};
        slider_row(QStringLiteral("forward  (%1 m/s)").arg(m_body.max_forward_speed, 0, 'f', 2), -100, m_forward);
        slider_row(QStringLiteral("turn  (%1 rad/s, left +)").arg(m_body.max_turn_rate, 0, 'f', 2), -100, m_turn);
        slider_row(QStringLiteral("voice  (%1)").arg(m_body.max_vocalisation_strength, 0, 'f', 2), 0, m_voice);
        m_voice->setValue(100);
        auto* const buttons{new QHBoxLayout{}};
        auto* const call{new QPushButton{QStringLiteral("call  [Space]"), this}};
        call->setFocusPolicy(Qt::NoFocus);
        connect(call, &QPushButton::clicked, this, [this] {
            m_call_pending = true;
        });
        auto* const brake{new QPushButton{QStringLiteral("brake  [X]"), this}};
        brake->setFocusPolicy(Qt::NoFocus);
        connect(brake, &QPushButton::clicked, this, [this] {
            this->brake();
        });
        m_intent_label = new QLabel{this};
        buttons->addWidget(call);
        buttons->addWidget(brake);
        buttons->addWidget(m_intent_label, 1);
        rows->addLayout(buttons);
        column->addWidget(controls);

        m_status = new QLabel{QStringLiteral("senses: none yet   waiting for the first tick"), this};
        column->addWidget(m_status);

        updateHeader();
        m_poll = new QTimer{this};
        m_poll->setTimerType(Qt::PreciseTimer);
        connect(m_poll, &QTimer::timeout, this, &PanelWindow::poll);
        // A little faster than the Grid ticks (31.25 ms), so no tick waits a whole poll for a word.
        m_poll->start(25);
        // The window opens at what its views ask for, bounded by the screen: a fixed size
        // left the rightmost view - the feel, the chain now in it - clipped at its edge.
        const QScreen* const screen{QGuiApplication::primaryScreen()};
        resize(screen != nullptr ? sizeHint().boundedTo(screen->availableGeometry().size()) : sizeHint());
    }

    PanelWindow::~PanelWindow() = default;

    void PanelWindow::stopPolling()
    {
        if (m_poll != nullptr) {
            m_poll->stop();
        }
    }

    void PanelWindow::setPollIntervalMs(const int milliseconds)
    {
        m_poll->start(std::max(1, milliseconds));
    }

    QString PanelWindow::statusText() const
    {
        return m_status->text();
    }

    QString PanelWindow::headerText() const
    {
        return m_header->text();
    }

    WormLib::Intent PanelWindow::currentIntent() const noexcept
    {
        WormLib::Intent intent{};
        intent.forward_speed = fraction(*m_forward) * m_body.max_forward_speed;
        if (m_forward_held != m_reverse_held) {
            intent.forward_speed = m_forward_held ? m_body.max_forward_speed : -m_body.max_forward_speed;
        }
        intent.turn_rate = fraction(*m_turn) * m_body.max_turn_rate;
        if (m_left_held != m_right_held) {
            intent.turn_rate = m_left_held ? m_body.max_turn_rate : -m_body.max_turn_rate;
        }
        intent.vocalisation = m_call_pending ? fraction(*m_voice) * m_body.max_vocalisation_strength : 0.0f;
        return intent;
    }

    void PanelWindow::updateHeader()
    {
        QString text{QStringLiteral("creature %1   ").arg(m_body.creature_id)};
        if (m_body.segment_count > 0u) {
            text += QStringLiteral("a chain of %1 segment(s), %2 m apart   ").arg(m_body.segment_count).arg(m_body.segment_spacing, 0, 'f', 2);
        }
        text += QStringLiteral("%1 eye(s), %2 ear(s), up to %3 contact(s)   tick %4 ms")
                    .arg(m_body.eye_count)
                    .arg(m_body.ear_count)
                    .arg(m_body.max_contact_count)
                    .arg(m_body.nominal_dt_seconds * 1000.0f, 0, 'f', 2);
        if ((m_body.eyes_dropped > 0u) || (m_body.ears_dropped > 0u)) {
            text += QStringLiteral("   <span style=\"color:#e879f9\">%1 eye(s) and %2 ear(s) beyond the seam's capacity are not shown</span>")
                        .arg(m_body.eyes_dropped)
                        .arg(m_body.ears_dropped);
        }
        m_header->setText(text);
    }

    void PanelWindow::poll()
    {
        if (m_mailbox.takeSenses(m_senses, m_seen)) {
            m_silent_polls = 0u;
            for (qsizetype index{0}; index < m_eyes.size(); ++index) {
                if (static_cast<std::uint32_t>(index) < m_senses.eye_count) {
                    m_eyes[index]->showView(m_senses.eyes[index]);
                }
            }
            for (qsizetype index{0}; index < m_ears.size(); ++index) {
                if (static_cast<std::uint32_t>(index) < m_senses.ear_count) {
                    m_ears[index]->showView(m_senses.ears[index], m_senses.tick);
                }
            }
            m_feel->showSenses(m_senses);
        } else if (m_silent_polls < 1000000u) {
            ++m_silent_polls;
        }

        const WormLib::Intent intent{currentIntent()};
        m_mailbox.offerIntent(intent);
        ++m_offered;
        m_call_pending = false;

        m_intent_label->setText(QStringLiteral("asking: forward %1 m/s  turn %2 rad/s  voice %3")
                .arg(intent.forward_speed, 0, 'f', 2)
                .arg(intent.turn_rate, 0, 'f', 2)
                .arg(intent.vocalisation, 0, 'f', 2));
        QString status{QStringLiteral("senses: tick %1, generation %2   intents offered: %3").arg(m_senses.tick).arg(m_seen).arg(m_offered)};
        if (m_seen == 0u) {
            status += QStringLiteral("   waiting for the first tick");
        } else if (m_silent_polls > 40u) {
            status += QStringLiteral("   <span style=\"color:#e879f9\">the Grid has been silent for %1 polls</span>").arg(m_silent_polls);
        }
        m_status->setText(status);
    }

    void PanelWindow::brake()
    {
        m_forward->setValue(0);
        m_turn->setValue(0);
        m_forward_held = false;
        m_reverse_held = false;
        m_left_held = false;
        m_right_held = false;
    }

    void PanelWindow::keyPressEvent(QKeyEvent* const event)
    {
        if (event->isAutoRepeat()) {
            event->accept();
            return;
        }
        switch (event->key()) {
        case Qt::Key_W:
            m_forward_held = true;
            break;
        case Qt::Key_S:
            m_reverse_held = true;
            break;
        case Qt::Key_A:
            m_left_held = true;
            break;
        case Qt::Key_D:
            m_right_held = true;
            break;
        case Qt::Key_Space:
            m_call_pending = true;
            break;
        case Qt::Key_X:
            brake();
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
        }
        event->accept();
    }

    void PanelWindow::keyReleaseEvent(QKeyEvent* const event)
    {
        if (event->isAutoRepeat()) {
            event->accept();
            return;
        }
        switch (event->key()) {
        case Qt::Key_W:
            m_forward_held = false;
            break;
        case Qt::Key_S:
            m_reverse_held = false;
            break;
        case Qt::Key_A:
            m_left_held = false;
            break;
        case Qt::Key_D:
            m_right_held = false;
            break;
        default:
            QWidget::keyReleaseEvent(event);
            return;
        }
        event->accept();
    }

} // namespace PanelLib
