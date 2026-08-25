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
#include <QtGui/QKeyEvent>
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

    void EarView::showView(const WormLib::EarSnapshot& view)
    {
        m_view = view;
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

        // The arrivals: onset as a column, radial velocity as a colour - approaching cyan, receding orange.
        for (std::uint32_t arrival{0u}; arrival < m_view.arrival_count; ++arrival) {
            const TglArrival& a{m_view.arrivals[arrival]};
            if (!(m_station.bin_seconds > 0.0f)) {
                break;
            }
            const double column{a.onset_seconds / m_station.bin_seconds};
            if ((column < 0.0) || (column > bins)) {
                continue;
            }
            const double x{grid.left() + (column * cell_w)};
            painter.setPen(QPen{a.radial_velocity < 0.0f ? CYAN : ORANGE, 2.0});
            painter.drawLine(QPointF{x, grid.top() - 4.0}, QPointF{x, grid.bottom() + 4.0});
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

    FeelView::FeelView(QWidget* parent) :
        QWidget(parent)
    {
        setMinimumSize(420, 260);
    }

    void FeelView::showSenses(const WormLib::SensesSnapshot& senses)
    {
        m_senses = senses;
        m_have_senses = true;
        update();
    }

    QSize FeelView::sizeHint() const
    {
        return {340, 260};
    }

    void FeelView::paintEvent(QPaintEvent* const event)
    {
        (void)event;
        QPainter painter{this};
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), GLASS);
        painter.setPen(INK);
        painter.drawText(QPointF{8.0, 15.0}, QStringLiteral("feel"));
        if (!m_have_senses) {
            painter.setPen(DIM);
            painter.drawText(QPointF{8.0, 34.0}, QStringLiteral("nothing sensed yet"));
            return;
        }
        const WormLib::SensesSnapshot& s{m_senses};
        const double line{15.0};
        double y{34.0};
        painter.setPen(DIM);
        painter.drawText(QPointF{8.0, y},
            QStringLiteral("forward %1 m/s  vertical %2 m/s  turn %3 rad/s")
                .arg(s.body_forward_speed, 0, 'f', 2)
                .arg(s.body_vertical_speed, 0, 'f', 2)
                .arg(s.body_turn_rate, 0, 'f', 2));
        y += line;
        painter.drawText(QPointF{8.0, y}, QStringLiteral("specific force %1 m/s2  |%2|").arg(vec3(s.specific_force)).arg(length3(s.specific_force), 0, 'f', 2));
        y += line;
        painter.drawText(QPointF{8.0, y}, QStringLiteral("angular velocity %1 rad/s").arg(vec3(s.angular_velocity)));
        y += line;
        painter.drawText(QPointF{8.0, y}, QStringLiteral("irradiance %1").arg(s.irradiance, 0, 'f', 3));
        y += line;
        painter.setPen(INK);
        painter.drawText(QPointF{8.0, y}, QStringLiteral("%1 contact(s)").arg(s.contact_count));
        if (s.contacts_dropped > 0u) {
            painter.setPen(MAGENTA);
            painter.drawText(QPointF{110.0, y}, QStringLiteral("+ %1 beyond the seam's capacity").arg(s.contacts_dropped));
        }
        y += line;

        // The body from above: forward up the page, right to the right; each contact where it is,
        // its normal drawn from it, the world's up shown as a dot.
        const QRectF plan{width() - 128.0, 26.0, 120.0, 120.0};
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
            if (y < height() - 4.0) {
                painter.drawText(QPointF{8.0, y}, row);
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
        m_feel = new FeelView{this};
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
        resize(1280, 720);
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
        QString text{QStringLiteral("creature %1   %2 eye(s), %3 ear(s), up to %4 contact(s)   tick %5 ms")
                .arg(m_body.creature_id)
                .arg(m_body.eye_count)
                .arg(m_body.ear_count)
                .arg(m_body.max_contact_count)
                .arg(m_body.nominal_dt_seconds * 1000.0f, 0, 'f', 2)};
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
                    m_ears[index]->showView(m_senses.ears[index]);
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
