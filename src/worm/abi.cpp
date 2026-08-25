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

    The panel's lifetime is pinned to the ABI's: the Qt thread starts in library_init and is joined
    before library_shutdown returns; a window opens at rez and is gone before derez returns, so no
    window ever outlives the worm whose mailbox it reads. program_tick never touches the panel.
*/

#define TGL_PROGRAM_IMPLEMENTATION
#include <tgl/tgl_program_abi.h>

#include "body.hpp"
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

    //! One rezzed creature: the worm, and the window that steers it (null when there is none).
    struct Rezzed {
        WormLib::Worm worm;
#if defined(RC_WORM_HAS_PANEL)
        PanelLib::Window* window{nullptr};
#endif
    };

    void libraryInit(const TglLibraryInfo* info) TGL_NOEXCEPT
    {
        if (info != nullptr) {
            g_nominal_dt_seconds = info->nominal_dt_seconds;
        }
#if defined(RC_WORM_HAS_PANEL)
        // The Qt thread, up before the first rez; a runner with no display to draw into (a
        // headless Linux) answers false and the worm stands unsteered, which is a worm still.
        (void)PanelLib::start();
#endif
    }

    TglProgram* programRez(const TglCreatureDesc* desc, TglRenderModel* model) TGL_NOEXCEPT
    {
        if (desc == nullptr) {
            return nullptr;
        }
        try {
            // The body: the icosahedron, lent to the Grid from the worm's own storage for the
            // length of this call, exactly as the ABI states. The Grid copies before returning.
            if (model != nullptr) {
                WormLib::Body::theWorm().lend(*model);
            }
            Rezzed* const rezzed{new Rezzed{WormLib::Worm{*desc, g_nominal_dt_seconds}}};
#if defined(RC_WORM_HAS_PANEL)
            rezzed->window = PanelLib::open(rezzed->worm.mailbox(), rezzed->worm.body());
#endif
            g_rezzed.fetch_add(1u);
            return reinterpret_cast<TglProgram*>(rezzed);
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
            reinterpret_cast<Rezzed*>(program)->worm.tick(*senses, *actions);
        } catch (...) {
            // Worm::tick is noexcept itself; this is the boundary's own belt over those braces.
        }
    }

    void programDerez(TglProgram* program) TGL_NOEXCEPT
    {
        if (program == nullptr) {
            return;
        }
        Rezzed* const rezzed{reinterpret_cast<Rezzed*>(program)};
#if defined(RC_WORM_HAS_PANEL)
        // The window first, and to completion: it reads the worm's mailbox until it is gone.
        PanelLib::close(rezzed->window);
        rezzed->window = nullptr;
#endif
        delete rezzed;
        g_rezzed.fetch_sub(1u);
    }

    void libraryShutdown() TGL_NOEXCEPT
    {
#if defined(RC_WORM_HAS_PANEL)
        // The Qt thread quits and is joined here, so nothing of the panel's runs after this
        // returns and the host may unload the library.
        PanelLib::stop();
#endif
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
