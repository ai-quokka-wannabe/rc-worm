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

/*
    The C boundary: the one exported symbol and the five functions behind it, exactly as the Program
    ABI header states them. Nothing unwinds across this file in either direction - every function
    is noexcept and the ones that run C++ do so inside a catch (...) - and the Grid's contract is
    kept to the letter: a Program touches its state in program_derez and never after
    library_shutdown.
*/

#define TGL_PROGRAM_IMPLEMENTATION
#include <tgl/tgl_program_abi.h>

#include "worm.hpp"

#if defined(RC_WORM_HAS_PANEL)
#include <panel/panel.hpp>
#endif

#include <atomic>
#include <cstdint>
#include <new>

namespace
{

    //! The tick length the Grid declared, for the worm's own reckoning. Fixed for the run.
    float g_nominal_dt_seconds{0.0f};

    //! Creatures rezzed and not yet derezzed: what library_shutdown must find at zero.
    std::atomic<std::uint32_t> g_rezzed{0u};

    //! The Qt the panel was built against, or an empty string for a worm built without one.
    const char* g_panel_qt{""};

    void libraryInit(const TglLibraryInfo* info) TGL_NOEXCEPT
    {
        if (info != nullptr) {
            g_nominal_dt_seconds = info->nominal_dt_seconds;
        }
#if defined(RC_WORM_HAS_PANEL)
        // Etape 2: the kit, proven at load - the panel answers from inside the host process.
        g_panel_qt = PanelLib::qtVersion();
#endif
    }

    TglProgram* programRez(const TglCreatureDesc* desc, TglRenderModel* model) TGL_NOEXCEPT
    {
        // Etape 2: no body yet. The Grid zeroed the model, and a zeroed model is a legitimate,
        // invisible body. Etape 3 fills it.
        (void)model;
        if (desc == nullptr) {
            return nullptr;
        }
        try {
            WormLib::Worm* const worm{new WormLib::Worm{*desc}};
            g_rezzed.fetch_add(1u);
            return reinterpret_cast<TglProgram*>(worm);
        } catch (...) {
            // Out of memory is the only thing that can throw here, and a refusal is the honest
            // answer to it: the Grid treats NULL as "this Program cannot take this body".
            return nullptr;
        }
    }

    void programTick(TglProgram* program, const TglSenses* senses, TglActions* actions) TGL_NOEXCEPT
    {
        if ((program == nullptr) || (senses == nullptr) || (actions == nullptr)) {
            return;
        }
        try {
            reinterpret_cast<WormLib::Worm*>(program)->tick(*senses, *actions);
        } catch (...) {
            // Worm::tick is noexcept itself; this is the boundary's own belt over those braces.
        }
    }

    void programDerez(TglProgram* program) TGL_NOEXCEPT
    {
        if (program == nullptr) {
            return;
        }
        delete reinterpret_cast<WormLib::Worm*>(program);
        g_rezzed.fetch_sub(1u);
    }

    void libraryShutdown() TGL_NOEXCEPT
    {
        // Every creature was derezzed before this, or the Grid broke its word; either way, nothing
        // of the worm's is touched here. The counter is reset so a reloaded library starts clean.
        g_rezzed.store(0u);
        g_nominal_dt_seconds = 0.0f;
    }

    const TglProgramVTable g_vtable{TGL_PROGRAM_VTABLE_HEADER, libraryInit, programRez, programTick, programDerez, libraryShutdown};

} // namespace

// Declared extern "C" by the header; defined here with the same linkage and the export applied.
const TglProgramVTable* tglGetProgramVTable(const std::uint32_t abi_version) TGL_NOEXCEPT
{
    if (abi_version != TGL_ABI_VERSION) {
        return nullptr;
    }
    return &g_vtable;
}
