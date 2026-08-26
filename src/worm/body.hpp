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

#include <cstddef>
#include <cstdint>
#include <vector>

/*!
    The worm's body: one rigid segment, an icosahedron, near-black mirror faces and a green neon
    tube along every edge. The shape lives in this Program (the Grid decides the senses, the
    Program brings the body - PROGRAM_INTERFACE.md), built once at first use from the golden
    ratio and a fixed orientation, and lent to the Grid through a `TglRenderModel` whose arrays
    point at the body's own storage - borrowed for the rez call, exactly as the ABI states.

    Vanilla C++20, no Qt: a body is arithmetic. docs/BODY.md carries the numbers and the why.
*/
namespace WormLib
{

    /*!
        The circumradius of the icosahedron, metres: the distance from the body's origin to any of
        its twelve vertices. A quarter metre makes the edge 0.263 m and the body about half a metre
        across - a creature the first body's eyes at z = -0.2 sit just inside the nose of.
    */
    inline constexpr float BODY_CIRCUMRADIUS{0.25f};

    //! The neon tubes' radius, metres, and how far their centre line stands proud of the edge.
    inline constexpr float NEON_RADIUS{0.008f};
    inline constexpr float NEON_PROUD{0.004f};

    //! Which material each triangle wears: the shell, or the neon.
    inline constexpr std::uint32_t MATERIAL_SHELL{0u};
    inline constexpr std::uint32_t MATERIAL_NEON{1u};

    /*!
        The chain (the owner's ruling, 2026-08-26): a worm is icosahedra joined spike to spike.
        Eight segments, the wire's cap and a worm's worth. The joint is authored here: a neon stub
        stands out of the nose spike and out of its antipode, half a joint long each, so two
        consecutive segments' stubs meet tip to tip. The spacing between segments' origins is
        therefore the two spikes' distance apart (twice the circumradius - they are antipodal)
        plus two stub lengths.
    */
    inline constexpr std::uint32_t BODY_SEGMENTS{8u};
    inline constexpr float JOINT_STUB_LENGTH{0.03f};
    inline constexpr float SEGMENT_SPACING{(2.0f * BODY_CIRCUMRADIUS) + (2.0f * JOINT_STUB_LENGTH)};

    //! The body's arrays, owned here and lent to the Grid.
    class Body {
    public:
        //! The one body, built on first use - deterministic arithmetic, no randomness, same bytes every time.
        [[nodiscard]] static const Body& theWorm();

        //! Fills a model with pointers into this body's storage. The Grid copies before program_rez returns.
        void lend(TglRenderModel& model) const noexcept;

        [[nodiscard]] const std::vector<float>& positions() const noexcept
        {
            return m_positions;
        }

        [[nodiscard]] const std::vector<TglRenderTriangle>& triangles() const noexcept
        {
            return m_triangles;
        }

        [[nodiscard]] const std::vector<TglRenderMaterial>& materials() const noexcept
        {
            return m_materials;
        }

        [[nodiscard]] std::uint32_t vertexCount() const noexcept
        {
            return static_cast<std::uint32_t>(m_positions.size() / 3u);
        }

        //! The largest |coordinate| over every vertex, metres: what the world's extent rule judges.
        [[nodiscard]] float extent() const noexcept;

        //! The lowest y over every vertex: where the world stands the body, its origin this far above the floor.
        [[nodiscard]] float lowest() const noexcept;

        //! The two joint spikes: the nose vertex and its antipode, as indices into the shell.
        [[nodiscard]] std::uint32_t noseVertex() const noexcept
        {
            return m_nose;
        }

        [[nodiscard]] std::uint32_t tailVertex() const noexcept
        {
            return m_tail;
        }

    private:
        Body();

        std::vector<float> m_positions;
        std::vector<TglRenderTriangle> m_triangles;
        std::vector<TglRenderMaterial> m_materials;
        std::uint32_t m_nose{0u};
        std::uint32_t m_tail{0u};
    };

} // namespace WormLib
