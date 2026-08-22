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
    The library as the Grid loads it: the one exported symbol, the versions it refuses, the table it
    returns, and the whole lifecycle through that table - init, rez, tick, derez, shutdown - under
    the conventions the Grid keeps (zeroed actions, zeroed model, NULL as a refusal).
*/

#include <tgl/tgl_program_abi.h>

#include <testing/testing.hpp>

#include <cstdint>

TEST_CASE(the_one_symbol_refuses_every_version_but_its_own_and_names_its_table)
{
    TEST_CHECK(tglGetProgramVTable(TGL_ABI_VERSION - 1u) == nullptr);
    TEST_CHECK(tglGetProgramVTable(TGL_ABI_VERSION + 1u) == nullptr);
    TEST_CHECK(tglGetProgramVTable(0u) == nullptr);
    const TglProgramVTable* const table{tglGetProgramVTable(TGL_ABI_VERSION)};
    TEST_CHECK(table != nullptr);
    TEST_CHECK_EQUAL(table->struct_size, static_cast<std::uint32_t>(sizeof(TglProgramVTable)));
    TEST_CHECK_EQUAL(table->abi_version, static_cast<std::uint32_t>(TGL_ABI_VERSION));
    TEST_CHECK(table->library_init != nullptr);
    TEST_CHECK(table->program_rez != nullptr);
    TEST_CHECK(table->program_tick != nullptr);
    TEST_CHECK(table->program_derez != nullptr);
    TEST_CHECK(table->library_shutdown != nullptr);
    // Static storage: the same table every time, as the header requires.
    TEST_CHECK(tglGetProgramVTable(TGL_ABI_VERSION) == table);
}

TEST_CASE(the_whole_lifecycle_through_the_table_rezzes_ticks_and_derezzes_two_worms)
{
    const TglProgramVTable* const table{tglGetProgramVTable(TGL_ABI_VERSION)};
    TEST_CHECK(table != nullptr);
    const TglLibraryInfo info{.creature_count = 2u, .nominal_dt_seconds = 0.03125f};
    table->library_init(&info);

    TglCreatureDesc desc{};
    desc.creature_id = 7u;
    TglRenderModel model{};
    TglProgram* const first{table->program_rez(&desc, &model)};
    TEST_CHECK(first != nullptr);
    // Etape 2: no body offered - the model stays as the Grid zeroed it.
    TEST_CHECK_EQUAL(model.vertex_count, 0u);
    TEST_CHECK(model.vertex_positions == nullptr);
    desc.creature_id = 8u;
    TglProgram* const second{table->program_rez(&desc, &model)};
    TEST_CHECK(second != nullptr);
    TEST_CHECK(second != first);

    TglSenses senses{};
    senses.dt_seconds = info.nominal_dt_seconds;
    TglActions actions{};
    for (std::uint32_t tick{0u}; tick < 10u; ++tick) {
        senses.tick = 5u + tick;
        table->program_tick(first, &senses, &actions);
        table->program_tick(second, &senses, &actions);
        TEST_CHECK_EQUAL(actions.desired_forward_speed, 0.0f);
        TEST_CHECK_EQUAL(actions.desired_turn_rate, 0.0f);
        TEST_CHECK_EQUAL(actions.vocalisation_strength, 0.0f);
    }

    // A null anywhere is nothing, never a crash: the Grid never sends one, but a boundary that
    // survives a null is a boundary that was written with care.
    table->program_tick(nullptr, &senses, &actions);
    table->program_tick(first, nullptr, &actions);
    table->program_tick(first, &senses, nullptr);
    TEST_CHECK(table->program_rez(nullptr, &model) == nullptr);

    table->program_derez(first);
    table->program_derez(second);
    table->program_derez(nullptr);
    table->library_shutdown();
}

int main()
{
    return static_cast<int>(TestingLib::runAll());
}
