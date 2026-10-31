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

/*!
    Draws docs/figures/gait.svg: the angle the worm's own gait asks of every servo, tick by tick,
    while a scripted User rests, drives forward, turns left and lets go. The numbers come from
    WormLib::Gait itself - the same class the Program ticks - and nothing here models the gait a
    second time. The SVG is written by hand, coordinates rounded to a tenth of a pixel, so a rerun
    on the same build writes the same bytes.

    Usage: gait_figure <output.svg>
*/

#include <body.hpp>
#include <gait.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{

    //! The Grid's tick: 32 Hz.
    constexpr float TICK_SECONDS{1.0f / 32.0f};

    //! The swing bound the Grid grants a chain today (tron-grid-lite's roster), radians.
    constexpr float GRID_MAX_JOINT_ANGLE{0.9f};

    constexpr std::uint32_t JOINTS{WormLib::BODY_SEGMENTS - 1u};
    constexpr std::uint32_t TICKS{256u}; // eight seconds

    //! One stretch of the script: the User's forward and turn, as fractions of the body's bounds.
    struct Stretch {
        std::uint32_t first_tick;
        float forward;
        float turn;
        const char* label;
        const char* tint;
    };

    constexpr std::array<Stretch, 4> SCRIPT{{
        {0u, 0.0f, 0.0f, "rest", ""},
        {16u, 1.0f, 0.0f, "full forward", "#00e5ff"},
        {112u, 1.0f, 1.0f, "full forward + full turn left", "#ff3df2"},
        {192u, 0.0f, 0.0f, "let go: the wave relaxes", ""},
    }};

    const Stretch& stretchAt(const std::uint32_t tick)
    {
        std::size_t index{0u};
        for (std::size_t i{0u}; i < SCRIPT.size(); ++i) {
            if (tick >= SCRIPT[i].first_tick) {
                index = i;
            }
        }
        return SCRIPT[index];
    }

    // Canvas.
    constexpr double WIDTH{960.0};
    constexpr double HEIGHT{676.0};
    constexpr double PLOT_LEFT{150.0};
    constexpr double PLOT_RIGHT{930.0};
    constexpr double ROWS_TOP{100.0};
    constexpr double ROW_HEIGHT{54.0};
    constexpr double ROW_SCALE{22.0}; // pixels for the full swing bound
    constexpr double INPUT_TOP{ROWS_TOP + (ROW_HEIGHT * JOINTS) + 20.0};
    constexpr double INPUT_HEIGHT{80.0};
    constexpr double INPUT_SCALE{30.0}; // pixels for a whole word, 1.0

    const char* const BACKGROUND{"#05080d"};
    const char* const GRID_LINE{"#1c2a38"};
    const char* const TEXT_DIM{"#7f9bb0"};
    const char* const CYAN{"#00e5ff"};
    const char* const ORANGE{"#ff9e3d"};
    const char* const MAGENTA{"#ff3df2"};

    std::string num(const double value)
    {
        std::array<char, 32> buffer{};
        const int written{std::snprintf(buffer.data(), buffer.size(), "%.1f", value)};
        std::string text{buffer.data(), written > 0 ? static_cast<std::size_t>(written) : 0u};
        return text == "-0.0" ? std::string{"0.0"} : text;
    }

    double tickX(const double tick)
    {
        return PLOT_LEFT + ((PLOT_RIGHT - PLOT_LEFT) * tick / static_cast<double>(TICKS));
    }

    double rowCentre(const std::uint32_t joint)
    {
        return ROWS_TOP + (ROW_HEIGHT * joint) + (ROW_HEIGHT / 2.0);
    }

    std::string text(const double x, const double y, const char* const fill, const double size, const std::string& body, const char* const anchor = "start")
    {
        return "<text x=\"" + num(x) + "\" y=\"" + num(y) + "\" fill=\"" + fill + "\" font-size=\"" + num(size) + "\" text-anchor=\"" + anchor + "\">" + body
            + "</text>\n";
    }

    std::string line(const double x1, const double y1, const double x2, const double y2, const char* const stroke, const char* const extra = "")
    {
        return "<line x1=\"" + num(x1) + "\" y1=\"" + num(y1) + "\" x2=\"" + num(x2) + "\" y2=\"" + num(y2) + "\" stroke=\"" + stroke + "\"" + extra + "/>\n";
    }

} // namespace

int main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << "usage: gait_figure <output.svg>\n";
        return 2;
    }

    // The run: the real gait, ticked as the Program ticks it, every target kept.
    std::vector<std::array<float, JOINTS>> targets(TICKS);
    WormLib::Gait gait{JOINTS, GRID_MAX_JOINT_ANGLE};
    for (std::uint32_t tick{0u}; tick < TICKS; ++tick) {
        const Stretch& stretch{stretchAt(tick)};
        float out[TGL_SEGMENTS_MAX - 1u]{};
        gait.tick(stretch.forward, stretch.turn, TICK_SECONDS, out);
        for (std::uint32_t joint{0u}; joint < JOINTS; ++joint) {
            targets[tick][joint] = out[joint];
        }
    }

    std::string svg;
    svg += "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" + num(WIDTH) + "\" height=\"" + num(HEIGHT) + "\" viewBox=\"0 0 " + num(WIDTH) + " " + num(HEIGHT)
        + "\" font-family=\"Consolas, 'DejaVu Sans Mono', monospace\">\n";
    svg += "<!-- Generated by tools/gait_figure from WormLib::Gait; do not edit by hand. -->\n";
    svg += "<title>rc-worm gait: the angle each servo is asked to hold, tick by tick</title>\n";
    svg += "<defs><filter id=\"glow\" x=\"-5%\" y=\"-50%\" width=\"110%\" height=\"200%\"><feGaussianBlur stdDeviation=\"1.5\" result=\"blur\"/>"
           "<feMerge><feMergeNode in=\"blur\"/><feMergeNode in=\"SourceGraphic\"/></feMerge></filter></defs>\n";
    svg += "<rect width=\"" + num(WIDTH) + "\" height=\"" + num(HEIGHT) + "\" rx=\"12\" fill=\"" + BACKGROUND + "\"/>\n";

    svg += text(24.0, 36.0, CYAN, 20.0, "The gait: a travelling wave of servo targets");
    svg += text(24.0, 60.0, TEXT_DIM, 12.5,
        "WormLib::Gait ticked at 32 Hz for the worm's 7 joints: each row is one joint's servo target within the &#177;0.9 rad swing bound.");

    // The script's stretches: tinted bands and their names.
    const double bands_bottom{INPUT_TOP + INPUT_HEIGHT};
    for (std::size_t i{0u}; i < SCRIPT.size(); ++i) {
        const double x1{tickX(SCRIPT[i].first_tick)};
        const double x2{tickX(i + 1u < SCRIPT.size() ? SCRIPT[i + 1u].first_tick : TICKS)};
        if (SCRIPT[i].tint[0] != '\0') {
            svg += "<rect x=\"" + num(x1) + "\" y=\"" + num(ROWS_TOP - 14.0) + "\" width=\"" + num(x2 - x1) + "\" height=\"" + num(bands_bottom - ROWS_TOP + 14.0)
                + "\" fill=\"" + SCRIPT[i].tint + "\" fill-opacity=\"0.07\"/>\n";
        }
        svg += line(x1, ROWS_TOP - 14.0, x1, bands_bottom, GRID_LINE);
        svg += text((x1 + x2) / 2.0, ROWS_TOP - 2.0, SCRIPT[i].tint[0] != '\0' ? SCRIPT[i].tint : TEXT_DIM, 12.0, SCRIPT[i].label, "middle");
    }

    // One row per joint: the bound, the zero, the target, and every crest.
    for (std::uint32_t joint{0u}; joint < JOINTS; ++joint) {
        const double centre{rowCentre(joint)};
        svg += line(PLOT_LEFT, centre - ROW_SCALE, PLOT_RIGHT, centre - ROW_SCALE, GRID_LINE, " stroke-dasharray=\"3 5\"");
        svg += line(PLOT_LEFT, centre, PLOT_RIGHT, centre, GRID_LINE);
        svg += line(PLOT_LEFT, centre + ROW_SCALE, PLOT_RIGHT, centre + ROW_SCALE, GRID_LINE, " stroke-dasharray=\"3 5\"");
        std::string name{"joint " + std::to_string(joint)};
        if (joint == 0u) {
            name += " (head)";
        } else if (joint == JOINTS - 1u) {
            name += " (tail)";
        }
        svg += text(PLOT_LEFT - 12.0, centre + 4.0, TEXT_DIM, 12.0, name, "end");

        svg += "<polyline fill=\"none\" stroke=\"" + std::string{CYAN} + "\" stroke-width=\"1.6\" stroke-linejoin=\"round\" filter=\"url(#glow)\" points=\"";
        for (std::uint32_t tick{0u}; tick < TICKS; ++tick) {
            svg += (tick == 0u ? "" : " ") + num(tickX(tick)) + "," + num(centre - (ROW_SCALE * targets[tick][joint] / GRID_MAX_JOINT_ANGLE));
        }
        svg += "\"/>\n";

        // Crests are marked on the straight run only, away from a change of word: there a local
        // maximum is the wave's own, not the kink a new word puts in the ramp.
        for (std::uint32_t tick{1u}; tick + 1u < TICKS; ++tick) {
            const Stretch& stretch{stretchAt(tick)};
            if (stretch.forward == 0.0f || stretch.turn != 0.0f || &stretchAt(tick - 1u) != &stretch || &stretchAt(tick + 1u) != &stretch) {
                continue;
            }
            const float before{targets[tick - 1u][joint]};
            const float here{targets[tick][joint]};
            const float after{targets[tick + 1u][joint]};
            if (here > before && here >= after && here > 0.15f) {
                svg += "<circle cx=\"" + num(tickX(tick)) + "\" cy=\"" + num(centre - (ROW_SCALE * here / GRID_MAX_JOINT_ANGLE)) + "\" r=\"2.6\" fill=\"" + ORANGE
                    + "\"/>\n";
            }
        }
    }

    // The User's word, beneath: forward and turn as fractions of the body's bounds.
    const double input_centre{INPUT_TOP + (INPUT_HEIGHT / 2.0)};
    svg += line(PLOT_LEFT, input_centre, PLOT_RIGHT, input_centre, GRID_LINE);
    svg += text(PLOT_LEFT - 12.0, input_centre - 8.0, CYAN, 12.0, "forward", "end");
    svg += text(PLOT_LEFT - 12.0, input_centre + 8.0, MAGENTA, 12.0, "turn", "end");
    svg += text(PLOT_LEFT - 12.0, input_centre + 24.0, TEXT_DIM, 11.0, "the User's word", "end");
    const auto word_trace{[&](const bool forward, const char* const stroke, const char* const dash) {
        std::string out{"<polyline fill=\"none\" stroke=\"" + std::string{stroke} + "\" stroke-width=\"2\"" + dash + " points=\""};
        for (std::uint32_t tick{0u}; tick <= TICKS; ++tick) {
            const Stretch& stretch{stretchAt(tick < TICKS ? tick : TICKS - 1u)};
            const double value{forward ? stretch.forward : stretch.turn};
            out += (tick == 0u ? "" : " ") + num(tickX(tick)) + "," + num(input_centre - (INPUT_SCALE * value));
        }
        return out + "\"/>\n";
    }};
    svg += word_trace(true, CYAN, "");
    svg += word_trace(false, MAGENTA, " stroke-dasharray=\"6 3\"");

    // Time.
    for (std::uint32_t second{0u}; second <= TICKS / 32u; ++second) {
        const double x{tickX(second * 32.0)};
        svg += line(x, bands_bottom, x, bands_bottom + 5.0, TEXT_DIM);
        svg += text(x, bands_bottom + 18.0, TEXT_DIM, 11.0, std::to_string(second) + " s", "middle");
    }

    // The key.
    const double key_y{HEIGHT - 40.0};
    svg += line(24.0, key_y - 4.0, 44.0, key_y - 4.0, CYAN, " stroke-width=\"2\"");
    svg += text(52.0, key_y, TEXT_DIM, 11.5, "servo target");
    svg += "<circle cx=\"170\" cy=\"" + num(key_y - 4.0) + "\" r=\"3\" fill=\"" + ORANGE + "\"/>\n";
    svg += text(180.0, key_y, TEXT_DIM, 11.5, "a crest of the straight run: each joint meets it a quarter cycle after the one before - the wave travels tailward");
    svg += line(24.0, key_y + 16.0, 44.0, key_y + 16.0, MAGENTA, " stroke-width=\"2\" stroke-dasharray=\"6 3\"");
    svg += text(52.0, key_y + 20.0, TEXT_DIM, 11.5, "turning left adds the same bend to every joint; the swing bound clips the sum into flat tops");

    svg += "</svg>\n";

    std::ofstream file{argv[1], std::ios::binary | std::ios::trunc};
    file << svg;
    file.close();
    if (!file) {
        std::cerr << "gait_figure: could not write " << argv[1] << "\n";
        return 1;
    }
    std::cout << "gait_figure: wrote " << argv[1] << "\n";
    return 0;
}
