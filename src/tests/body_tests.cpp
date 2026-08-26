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
    The body, held to every rule the Grid and the world judge a model by, before either sees it:
    the counts under the wire's caps, every index inside its array, every float finite and
    normal, every triangle with area and wound outward, the extent under the world's, the shell
    a mirror and the tubes a light - and the same bytes every time it is asked for.
*/

#include <body.hpp>

#include <testing/testing.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{

    // The wire's caps and the world's extent, as Link and Master Control state them.
    constexpr std::uint32_t REZ_MAX_VERTICES{1024u};
    constexpr std::uint32_t REZ_MAX_TRIANGLES{2048u};
    constexpr std::uint32_t REZ_MAX_MATERIALS{16u};
    constexpr float BODY_MAX_EXTENT{4.0f};

    [[nodiscard]] bool normalFloat(const float value)
    {
        return std::isfinite(value) && ((value == 0.0f) || std::isnormal(value));
    }

}

TEST_CASE(the_body_is_an_icosahedron_with_a_tube_on_every_edge)
{
    const WormLib::Body& body{WormLib::Body::theWorm()};
    // Twelve shell vertices and twenty faces; thirty edges, each a prism of six vertices and
    // six triangles; two joint stubs, the same prism each.
    TEST_CHECK_EQUAL(body.vertexCount(), 12u + (30u * 6u) + (2u * 6u));
    TEST_CHECK_EQUAL(static_cast<std::uint32_t>(body.triangles().size()), 20u + (30u * 6u) + (2u * 6u));
    TEST_CHECK_EQUAL(static_cast<std::uint32_t>(body.materials().size()), 2u);
    TEST_CHECK(body.vertexCount() <= REZ_MAX_VERTICES);
    TEST_CHECK(body.triangles().size() <= REZ_MAX_TRIANGLES);
    TEST_CHECK(body.materials().size() <= REZ_MAX_MATERIALS);

    // Every shell vertex lies on the circumsphere: an icosahedron, not something else.
    for (std::uint32_t index{0u}; index < 12u; ++index) {
        const float x{body.positions()[index * 3u]};
        const float y{body.positions()[(index * 3u) + 1u]};
        const float z{body.positions()[(index * 3u) + 2u]};
        TEST_CHECK_CLOSE(std::sqrt((x * x) + (y * y) + (z * z)), WormLib::BODY_CIRCUMRADIUS, 1e-5f);
    }
}

TEST_CASE(the_body_passes_every_rule_the_grid_and_the_world_judge_a_model_by)
{
    const WormLib::Body& body{WormLib::Body::theWorm()};
    const auto& positions{body.positions()};
    for (const float value : positions) {
        TEST_CHECK(normalFloat(value));
    }
    TEST_CHECK(body.extent() < BODY_MAX_EXTENT);
    TEST_CHECK(body.extent() > 0.2f);

    for (const TglRenderTriangle& triangle : body.triangles()) {
        for (const std::uint32_t vertex : triangle.vertices) {
            TEST_CHECK(vertex < body.vertexCount());
        }
        TEST_CHECK(triangle.material < body.materials().size());
        // Area, and outward winding: the face normal points away from the origin, which is
        // where the body's centre is - the tracer reads such a face as a surface seen from
        // outside, and the world's hull builder is handed a convex shell.
        const float* const a{&positions[triangle.vertices[0] * 3u]};
        const float* const b{&positions[triangle.vertices[1] * 3u]};
        const float* const c{&positions[triangle.vertices[2] * 3u]};
        const float e1[3]{b[0] - a[0], b[1] - a[1], b[2] - a[2]};
        const float e2[3]{c[0] - a[0], c[1] - a[1], c[2] - a[2]};
        const float n[3]{(e1[1] * e2[2]) - (e1[2] * e2[1]), (e1[2] * e2[0]) - (e1[0] * e2[2]), (e1[0] * e2[1]) - (e1[1] * e2[0])};
        const float area2{(n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2])};
        TEST_CHECK(area2 > 1e-12f);
        float centroid[3]{(a[0] + b[0] + c[0]) / 3.0f, (a[1] + b[1] + c[1]) / 3.0f, (a[2] + b[2] + c[2]) / 3.0f};
        if (triangle.material == WormLib::MATERIAL_NEON) {
            // A tube's faces look away from its own centre line: the prism's six vertices
            // average to a point on that line.
            const std::uint32_t base{12u + (((triangle.vertices[0] - 12u) / 6u) * 6u)};
            float middle[3]{0.0f, 0.0f, 0.0f};
            for (std::uint32_t vertex{base}; vertex < base + 6u; ++vertex) {
                for (std::uint32_t axis{0u}; axis < 3u; ++axis) {
                    middle[axis] += positions[(vertex * 3u) + axis] / 6.0f;
                }
            }
            for (std::uint32_t axis{0u}; axis < 3u; ++axis) {
                centroid[axis] -= middle[axis];
            }
        }
        TEST_CHECK(((n[0] * centroid[0]) + (n[1] * centroid[1]) + (n[2] * centroid[2])) > 0.0f);
    }

    for (const TglRenderMaterial& material : body.materials()) {
        for (const float value : {material.colour[0], material.colour[1], material.colour[2], material.index_of_refraction, material.emission[0], material.emission[1],
                 material.emission[2], material.transmission}) {
            TEST_CHECK(std::isfinite(value));
        }
        TEST_CHECK(material.index_of_refraction > 0.0f);
        TEST_CHECK((material.transmission >= 0.0f) && (material.transmission <= 1.0f));
    }
    // The shell is a mirror and the tubes are a green light.
    TEST_CHECK_EQUAL(body.materials()[WormLib::MATERIAL_SHELL].emission[1], 0.0f);
    TEST_CHECK(body.materials()[WormLib::MATERIAL_NEON].emission[1] > body.materials()[WormLib::MATERIAL_NEON].emission[0]);
    TEST_CHECK(body.materials()[WormLib::MATERIAL_NEON].emission[1] > body.materials()[WormLib::MATERIAL_NEON].emission[2]);
}

TEST_CASE(the_body_rests_on_a_face_with_its_nose_forward_and_is_the_same_bytes_every_time)
{
    const WormLib::Body& body{WormLib::Body::theWorm()};
    // Face down: three shell vertices share the lowest height, within a hair, so the body
    // rests on a face rather than balancing on a point.
    float lowest{0.0f};
    for (std::uint32_t index{0u}; index < 12u; ++index) {
        lowest = std::min(lowest, body.positions()[(index * 3u) + 1u]);
    }
    std::uint32_t on_the_floor{0u};
    for (std::uint32_t index{0u}; index < 12u; ++index) {
        if (std::fabs(body.positions()[(index * 3u) + 1u] - lowest) < 1e-4f) {
            ++on_the_floor;
        }
    }
    TEST_CHECK_EQUAL(on_the_floor, 3u);
    // The tubes stand proud, so the world stands the body on a tube's rail, just below the face.
    TEST_CHECK(body.lowest() < lowest);
    TEST_CHECK(body.lowest() > lowest - 0.02f);

    // A nose: some shell vertex lies on the -Z axis (x = 0) at the waist, where the eyes look.
    bool nose{false};
    for (std::uint32_t index{0u}; index < 12u; ++index) {
        const float x{body.positions()[index * 3u]};
        const float z{body.positions()[(index * 3u) + 2u]};
        if ((std::fabs(x) < 1e-4f) && (z < -0.2f)) {
            nose = true;
        }
    }
    TEST_CHECK(nose);

    // The chain: eight segments joined stub to stub. The nose spike is on -Z, its antipode on +Z,
    // and the two stubs stand out of them along their own directions, half a joint each, so the
    // spacing is a diameter plus two stubs. The model says so when lent.
    TEST_CHECK_EQUAL(WormLib::BODY_SEGMENTS, 8u);
    TEST_CHECK_CLOSE(WormLib::SEGMENT_SPACING, (2.0f * WormLib::BODY_CIRCUMRADIUS) + (2.0f * WormLib::JOINT_STUB_LENGTH), 1e-6f);
    const float* const nose_at{&body.positions()[body.noseVertex() * 3u]};
    const float* const tail_at{&body.positions()[body.tailVertex() * 3u]};
    TEST_CHECK(nose_at[2] < -0.2f);
    TEST_CHECK(tail_at[2] > 0.2f);
    TEST_CHECK_CLOSE(nose_at[0] + tail_at[0], 0.0f, 1e-5f); // antipodal
    TEST_CHECK_CLOSE(nose_at[1] + tail_at[1], 0.0f, 1e-5f);
    TEST_CHECK_CLOSE(nose_at[2] + tail_at[2], 0.0f, 1e-5f);
    // The stubs are the last twelve vertices: every one within a stub's reach of its spike, and
    // the far end of each rail a stub length further out along the spike than the near end.
    const std::uint32_t stubs_from{12u + (30u * 6u)};
    for (std::uint32_t stub{0u}; stub < 2u; ++stub) {
        const float* const spike{stub == 0u ? nose_at : tail_at};
        const float spike_length{std::sqrt((spike[0] * spike[0]) + (spike[1] * spike[1]) + (spike[2] * spike[2]))};
        for (std::uint32_t rail{0u}; rail < 3u; ++rail) {
            const float* const near{&body.positions()[(stubs_from + (stub * 6u) + (rail * 2u)) * 3u]};
            const float* const far{&body.positions()[(stubs_from + (stub * 6u) + (rail * 2u) + 1u) * 3u]};
            const float near_along{((near[0] * spike[0]) + (near[1] * spike[1]) + (near[2] * spike[2])) / spike_length};
            const float far_along{((far[0] * spike[0]) + (far[1] * spike[1]) + (far[2] * spike[2])) / spike_length};
            TEST_CHECK_CLOSE(near_along, spike_length, 1e-4f);
            TEST_CHECK_CLOSE(far_along - near_along, WormLib::JOINT_STUB_LENGTH, 1e-4f);
        }
    }

    // Deterministic: the same body twice is the same bytes, and lending fills a model whole.
    const WormLib::Body& again{WormLib::Body::theWorm()};
    TEST_CHECK(&again == &body);
    TglRenderModel model{};
    body.lend(model);
    TEST_CHECK(model.vertex_positions == body.positions().data());
    TEST_CHECK_EQUAL(model.vertex_count, body.vertexCount());
    TEST_CHECK_EQUAL(model.triangle_count, static_cast<std::uint32_t>(body.triangles().size()));
    TEST_CHECK_EQUAL(model.material_count, 2u);
    TEST_CHECK_EQUAL(model.segment_count, WormLib::BODY_SEGMENTS);
    TEST_CHECK_CLOSE(model.segment_spacing, WormLib::SEGMENT_SPACING, 1e-6f);
    TEST_CHECK_EQUAL(model.padding0, 0u);
}

int main()
{
    return static_cast<int>(TestingLib::runAll());
}
