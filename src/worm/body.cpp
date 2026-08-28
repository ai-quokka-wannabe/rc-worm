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

#include "body.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace WormLib
{

    namespace
    {

        //! Three floats and the little arithmetic a body needs; nothing a library is owed for.
        struct V3 {
            float x, y, z;
        };

        [[nodiscard]] constexpr V3 operator+(const V3 a, const V3 b) noexcept
        {
            return {a.x + b.x, a.y + b.y, a.z + b.z};
        }

        [[nodiscard]] constexpr V3 operator-(const V3 a, const V3 b) noexcept
        {
            return {a.x - b.x, a.y - b.y, a.z - b.z};
        }

        [[nodiscard]] constexpr V3 operator*(const V3 a, const float s) noexcept
        {
            return {a.x * s, a.y * s, a.z * s};
        }

        [[nodiscard]] constexpr float dot(const V3 a, const V3 b) noexcept
        {
            return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
        }

        [[nodiscard]] constexpr V3 cross(const V3 a, const V3 b) noexcept
        {
            return {(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};
        }

        [[nodiscard]] V3 normalised(const V3 a) noexcept
        {
            const float length{std::sqrt(dot(a, a))};
            return a * (1.0f / length);
        }

        //! Rotate `p` about the unit axis `k` by `angle` radians (Rodrigues).
        [[nodiscard]] V3 rotated(const V3 p, const V3 k, const float angle) noexcept
        {
            const float c{std::cos(angle)};
            const float s{std::sin(angle)};
            return (p * c) + (cross(k, p) * s) + (k * (dot(k, p) * (1.0f - c)));
        }

        /*
            The regular icosahedron: the twelve vertices are the corners of three mutually
            perpendicular golden rectangles, (0, ±1, ±φ) and its two cyclic permutations. The
            twenty faces are the standard listing over that ordering, each wound counter-clockwise
            seen from outside, so every face normal points out of the body and the tracer's
            "entering" test reads the shell as a shell.
        */
        constexpr float PHI{1.618033988749895f};

        constexpr std::array<V3, 12> RAW_VERTICES{{
            {-1.0f, PHI, 0.0f},
            {1.0f, PHI, 0.0f},
            {-1.0f, -PHI, 0.0f},
            {1.0f, -PHI, 0.0f},
            {0.0f, -1.0f, PHI},
            {0.0f, 1.0f, PHI},
            {0.0f, -1.0f, -PHI},
            {0.0f, 1.0f, -PHI},
            {PHI, 0.0f, -1.0f},
            {PHI, 0.0f, 1.0f},
            {-PHI, 0.0f, -1.0f},
            {-PHI, 0.0f, 1.0f},
        }};

        constexpr std::array<std::array<std::uint32_t, 3>, 20> FACES{{
            {0, 11, 5},
            {0, 5, 1},
            {0, 1, 7},
            {0, 7, 10},
            {0, 10, 11},
            {1, 5, 9},
            {5, 11, 4},
            {11, 10, 2},
            {10, 7, 6},
            {7, 1, 8},
            {3, 9, 4},
            {3, 4, 2},
            {3, 2, 6},
            {3, 6, 8},
            {3, 8, 9},
            {4, 9, 5},
            {2, 4, 11},
            {6, 2, 10},
            {8, 6, 7},
            {9, 8, 1},
        }};

    } // namespace

    const Body& Body::theWorm()
    {
        static const Body body{};
        return body;
    }

    Body::Body()
    {
        // Scale to the circumradius, then orient: face 0 down (the body rests on a face, not a
        // point), then turn about the vertical so the vertex nearest the nose direction sits
        // exactly on -Z, where the first body's eyes look.
        const float raw_radius{std::sqrt(1.0f + (PHI * PHI))};
        std::array<V3, 12> vertices{};
        for (std::size_t index{0u}; index < vertices.size(); ++index) {
            vertices[index] = RAW_VERTICES[index] * (BODY_CIRCUMRADIUS / raw_radius);
        }

        // Face 0 down: rotate its outward normal onto -Y.
        {
            const auto& face{FACES[0]};
            const V3 normal{normalised(cross(vertices[face[1]] - vertices[face[0]], vertices[face[2]] - vertices[face[0]]))};
            const V3 down{0.0f, -1.0f, 0.0f};
            const V3 axis{cross(normal, down)};
            const float sine{std::sqrt(dot(axis, axis))};
            if (sine > 1e-6f) {
                const float angle{std::atan2(sine, dot(normal, down))};
                const V3 unit{axis * (1.0f / sine)};
                for (V3& vertex : vertices) {
                    vertex = rotated(vertex, unit, angle);
                }
            }
        }

        // The nose: of the vertices above the resting face, the one that can point most nearly
        // forward is turned exactly onto -Z (as seen from above); the icosahedron's symmetry
        // means the lowest face keeps lying flat through a turn about the vertical.
        {
            std::size_t nose{0u};
            float best{-1.0f};
            for (std::size_t index{0u}; index < vertices.size(); ++index) {
                const V3& v{vertices[index]};
                const float flat{std::sqrt((v.x * v.x) + (v.z * v.z))};
                // Prefer the vertices around the waist: the ones a nose can be, not a crown.
                if ((flat > best) && (v.y > -0.5f * BODY_CIRCUMRADIUS) && (v.y < 0.5f * BODY_CIRCUMRADIUS)) {
                    best = flat;
                    nose = index;
                }
            }
            const V3& n{vertices[nose]};
            // The turn about +Y by this angle takes (x, z) = r (sin a, -cos a) onto (0, -r).
            const float angle{std::atan2(n.x, -n.z)};
            const V3 up{0.0f, 1.0f, 0.0f};
            for (V3& vertex : vertices) {
                vertex = rotated(vertex, up, angle);
            }
        }

        // The pitch that puts the nose on -Z exactly, not only as seen from above. A waist
        // vertex of an icosahedron resting on a face sits 0.1876 circumradii above or below the
        // horizontal axis - 10.8 degrees - and the owner's report (2026-08-28) was that the spikes
        // of two neighbours must be one point, the pivot the chain bends around; so both spikes
        // go onto the axis the chain hinges along, and the antipode follows the nose for free.
        // The turn about +X by minus the nose's elevation takes (y, z) = r (sin b, -cos b) onto
        // (0, -r). The body no longer lies flat on a face: it stands on the one shell vertex the
        // tilt leaves lowest - a sharp spike on the Grid floor, as the ruling has it - and the
        // world stands it there by its lowest point as it always did.
        {
            std::size_t nose{0u};
            for (std::size_t index{1u}; index < vertices.size(); ++index) {
                if (vertices[index].z < vertices[nose].z) {
                    nose = index;
                }
            }
            const V3& n{vertices[nose]};
            const float elevation{std::atan2(n.y, -n.z)};
            const V3 right{1.0f, 0.0f, 0.0f};
            for (V3& vertex : vertices) {
                vertex = rotated(vertex, right, -elevation);
            }
        }

        // The two joint spikes, now that the body is oriented: the nose is the shell vertex on
        // -Z, its antipode the tail - an icosahedron's vertices come in antipodal pairs, so the
        // tail sits exactly a diameter behind the nose, through the origin, on +Z.
        {
            std::size_t nose{0u};
            for (std::size_t index{1u}; index < vertices.size(); ++index) {
                if (vertices[index].z < vertices[nose].z) {
                    nose = index;
                }
            }
            std::size_t tail{0u};
            float nearest{1.0e30f};
            for (std::size_t index{0u}; index < vertices.size(); ++index) {
                const V3 mirrored{vertices[index] + vertices[nose]};
                const float miss{dot(mirrored, mirrored)};
                if (miss < nearest) {
                    nearest = miss;
                    tail = index;
                }
            }
            m_nose = static_cast<std::uint32_t>(nose);
            m_tail = static_cast<std::uint32_t>(tail);
        }

        // The shell: twelve vertices, twenty faces.
        m_positions.reserve((12u + (30u * 6u) + (2u * 6u)) * 3u);
        for (const V3& v : vertices) {
            m_positions.insert(m_positions.end(), {v.x, v.y, v.z});
        }
        m_triangles.reserve(20u + (30u * 6u) + (2u * 6u));
        for (const auto& face : FACES) {
            m_triangles.push_back(TglRenderTriangle{.vertices = {face[0], face[1], face[2]}, .material = MATERIAL_SHELL});
        }

        // The neon: every edge once (each appears in two faces; keep it from the face that lists
        // it with the smaller index first, which every edge does in exactly one of its two
        // faces because the faces are wound consistently). A tube is a triangular prism around
        // the edge, its centre line standing a little proud of the shell along the mean of the
        // two neighbouring face normals, so it reads as a tube lying on the edge.
        for (std::size_t face_index{0u}; face_index < FACES.size(); ++face_index) {
            const auto& face{FACES[face_index]};
            for (std::size_t corner{0u}; corner < 3u; ++corner) {
                const std::uint32_t a{face[corner]};
                const std::uint32_t b{face[(corner + 1u) % 3u]};
                if (a > b) {
                    continue; // The other face lists this edge the other way round.
                }
                const V3 pa{vertices[a]};
                const V3 pb{vertices[b]};
                // The outward direction at the edge: away from the body's centre, which is the
                // origin - the mean of the two face normals points the same way for a convex
                // solid, and this is simpler and exact enough for a tube's placement.
                const V3 outward{normalised(pa + pb)};
                const V3 along{normalised(pb - pa)};
                const V3 side{normalised(cross(along, outward))};
                const std::uint32_t base{static_cast<std::uint32_t>(m_positions.size() / 3u)};
                // Three rails of the prism, each two vertices long, around a centre line proud
                // of the edge: one rail outward, two down-and-sideways.
                const std::array<V3, 3> spokes{{
                    outward * NEON_RADIUS,
                    (outward * (-0.5f * NEON_RADIUS)) + (side * (0.866f * NEON_RADIUS)),
                    (outward * (-0.5f * NEON_RADIUS)) - (side * (0.866f * NEON_RADIUS)),
                }};
                for (const V3& spoke : spokes) {
                    const V3 start{pa + (outward * NEON_PROUD) + spoke};
                    const V3 end{pb + (outward * NEON_PROUD) + spoke};
                    m_positions.insert(m_positions.end(), {start.x, start.y, start.z});
                    m_positions.insert(m_positions.end(), {end.x, end.y, end.z});
                }
                // Rails r0 = base+0/1, r1 = base+2/3, r2 = base+4/5; three side quads, two
                // triangles each, wound so their normals face away from the centre line.
                for (std::uint32_t rail{0u}; rail < 3u; ++rail) {
                    const std::uint32_t next{(rail + 1u) % 3u};
                    const std::uint32_t s0{base + (rail * 2u)};
                    const std::uint32_t e0{s0 + 1u};
                    const std::uint32_t s1{base + (next * 2u)};
                    const std::uint32_t e1{s1 + 1u};
                    m_triangles.push_back(TglRenderTriangle{.vertices = {s0, s1, e1}, .material = MATERIAL_NEON});
                    m_triangles.push_back(TglRenderTriangle{.vertices = {s0, e1, e0}, .material = MATERIAL_NEON});
                }
            }
        }

        // The joint stubs: out of the nose spike and out of its antipode to the joint tips, which
        // lie exactly on the body's axis - (0, 0, -JOINT_TIP_REACH) for the nose, its mirror for
        // the tail - as a triangular neon prism, the same prism the edges wear, so the joint
        // reads as the neon continuing off the body, straight along the axis now that the spikes
        // lie on it. Two consecutive segments meet at one tip, a pivot: spike, stub, tip, stub,
        // spike, all on one line.
        for (const std::uint32_t spike : {m_nose, m_tail}) {
            const V3 base{vertices[spike]};
            const V3 tip{0.0f, 0.0f, (spike == m_nose ? -1.0f : 1.0f) * JOINT_TIP_REACH};
            const V3 along{normalised(tip - base)};
            // A perpendicular pair around the stub's axis: the axis is never vertical, so up is
            // a safe partner for the first.
            const V3 outward{normalised(cross(along, V3{0.0f, 1.0f, 0.0f}))};
            const V3 side{cross(along, outward)};
            const std::uint32_t first{static_cast<std::uint32_t>(m_positions.size() / 3u)};
            const std::array<V3, 3> spokes{{
                outward * NEON_RADIUS,
                (outward * (-0.5f * NEON_RADIUS)) + (side * (0.866f * NEON_RADIUS)),
                (outward * (-0.5f * NEON_RADIUS)) - (side * (0.866f * NEON_RADIUS)),
            }};
            for (const V3& spoke : spokes) {
                const V3 start{base + spoke};
                const V3 end{tip + spoke};
                m_positions.insert(m_positions.end(), {start.x, start.y, start.z});
                m_positions.insert(m_positions.end(), {end.x, end.y, end.z});
            }
            for (std::uint32_t rail{0u}; rail < 3u; ++rail) {
                const std::uint32_t next{(rail + 1u) % 3u};
                const std::uint32_t s0{first + (rail * 2u)};
                const std::uint32_t e0{s0 + 1u};
                const std::uint32_t s1{first + (next * 2u)};
                const std::uint32_t e1{s1 + 1u};
                m_triangles.push_back(TglRenderTriangle{.vertices = {s0, s1, e1}, .material = MATERIAL_NEON});
                m_triangles.push_back(TglRenderTriangle{.vertices = {s0, e1, e0}, .material = MATERIAL_NEON});
            }
        }

        // The materials: a near-black mirror for the shell (the floor's reflectivity, a dark
        // green-tinged tint), and green neon for the tubes at the Grid's own neon intensities.
        m_materials.push_back(TglRenderMaterial{.colour = {0.06f, 0.08f, 0.07f}, .index_of_refraction = 2.4f, .emission = {0.0f, 0.0f, 0.0f}, .transmission = 0.0f});
        m_materials.push_back(TglRenderMaterial{.colour = {0.05f, 0.55f, 0.20f}, .index_of_refraction = 1.5f, .emission = {0.30f, 4.20f, 1.20f}, .transmission = 0.0f});
    }

    void Body::lend(TglRenderModel& model) const noexcept
    {
        model.vertex_positions = m_positions.data();
        model.triangles = m_triangles.data();
        model.materials = m_materials.data();
        model.vertex_count = vertexCount();
        model.triangle_count = static_cast<std::uint32_t>(m_triangles.size());
        model.material_count = static_cast<std::uint32_t>(m_materials.size());
        // The chain: eight of these, joined stub to stub.
        model.segment_count = BODY_SEGMENTS;
        model.segment_spacing = SEGMENT_SPACING;
        model.padding0 = 0u;
    }

    float Body::extent() const noexcept
    {
        float largest{0.0f};
        for (const float value : m_positions) {
            largest = std::max(largest, std::fabs(value));
        }
        return largest;
    }

    float Body::lowest() const noexcept
    {
        float lowest{0.0f};
        for (std::size_t index{1u}; index < m_positions.size(); index += 3u) {
            lowest = std::min(lowest, m_positions[index]);
        }
        return lowest;
    }

} // namespace WormLib
