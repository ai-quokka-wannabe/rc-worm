# TODO

Unpaused on 2026-08-23. The worm waited for the Grid to be able to hold it: the Program ABI
settled at version 6, Master Control steps a real world with real contacts, the wire carries a
body both ways and a Disk records a whole life. All of that is merged; the first inhabitant is
next. Each etape below lands as its own pull request and records what it decided.

## Etape 1 — the furniture

**Done (2026-08-23).** The organisation's settings, mirrored from the flagship as every
repository's are: lint and editor configuration, governance files, the CI shape (markdown
quick-checks feeding the `CI Success` gate the ruleset requires by name; the build matrix
arrives with the first code), `LICENCE` under its British name. Nothing here is improved on the
flagship's; a copy that drifts is the defect the mirroring exists to prevent.

## Etape 2 — the Program that loads

**Vanilla C++20 everywhere but the panel** (the owner's ruling, 2026-08-23): only `src/panel/`
may include Qt; the Program, the body, the ABI boundary and the thread seam use the standard
library alone, the seam crossed by plain structs, and the worm builds and loads with Qt
switched off (`RC_WORM_PANEL=OFF`), which CI proves beside the full build.

A C++20 shared library, `rc_worm`, built with CMake against the Program ABI header vendored
verbatim from the flagship (`libs/program-abi/include/tgl/tgl_program_abi.h`, its fingerprint
beside it, and a check that the vendored copy still matches the flagship's - the ABI is the
flagship's to move, and a drifted vendor copy is a Program the Grid refuses as stale, which is
the honest outcome but not one to discover at load time). `tglGetProgramVTable` refuses every
version but its own; `library_init`, `program_rez`, `program_tick`, `program_derez`,
`library_shutdown` exist, `noexcept` throughout, a Program that touches its state in
`program_derez` and never after `library_shutdown`. The first `program_tick` answers zeroes:
a worm that stands still is a worm that loaded. Proven by `TronGridLite --list-programs`
reporting it USABLE at ABI version 6, and by a `--program rc_worm` host run against Master
Control that rezzes it, ticks it, and leaves with BYE.

Built on **every compiler the flagship supports and every Qt kit on the owner's machine**
(the owner's ruling, 2026-08-23): `windows-msvc` (Qt `msvc2022_64`), `windows-clang-cl` (the
same kit - clang-cl is MSVC-ABI), `windows-mingw` (Qt `mingw_64`, the flagship's own MinGW
13.1), `windows-llvm-mingw` (Qt `llvm-mingw_64`, LLVM-MinGW 17), `linux-gcc` and `linux-clang`
(the distribution's Qt 6), with the flagship's ASan and TSan variants on Linux. The Grid is
MSVC, so the MinGW and LLVM-MinGW builds prove the C ABI between a Program and the Grid across
compilers, for real, rather than assuming it within one - and every preset's DLL must load in
the MSVC Grid alike.

## Etape 3 — the body

`program_rez` brings the worm's shape: a glass-neon mecha-worm as a `TglRenderModel` -
vertices, triangles and materials in the Grid's own continuous model (the mirror hull, the neon
of the segment rings; the materials the flagship's MATERIALS.md documents), validated whole by
the Grid and by Master Control (vertex count, triangle indices, extent under four metres, no
subnormal). **One rigid segment** for the first worm, per the flagship's Etape 13 decision: a
body that bends needs a solver able to bend it, and the world's physics is rigid and kinematic.
The Grid decides the senses - the first body's two eyes and two ears, placed by the
`TglCreatureDesc` the Grid hands over - and the worm's nose sits where those eyes are.

Tested without the Grid: the model's arrays are well-formed by the flagship's own rules (every
index inside its array, every float finite and normal, the extent bounded), and the Grid's
`copyValidatedModel` accepts it - run through `--list-programs` and a host run. The body's
extent, mass and sensor stations are written down in a `docs/BODY.md` with the reasoning, so
the later `brain-elegans` can wear the same body knowing what it is.

## Etape 4 — the panel

The User is the brain. A Qt Widgets window on the Program's own thread (a Program's window is
its own business; the Grid provides nothing and the tick never waits for a window):

- **The senses, as the worm has them.** Each eye's `TglEyeView` drawn as the image it is - the
  sample list laid out as the preset describes, not a camera render; each ear's `TglEarView` as
  the band-by-bin histogram it is, with the arrivals' onsets and radial velocities marked;
  the feel: grounded, specific force, every contact with its normal, depth and slip.
- **The controls.** Forward speed, turn rate, and the voice - the three members of
  `TglActions` - from keys or sliders, staged into the next `program_tick`'s answer. Nothing
  else: the ABI carries no more, and the panel shows no more than the worm knows.
- **The thread seam.** `program_tick` runs on the Grid's tick thread and must never block; the
  panel runs on the Qt thread. One mailbox each way, in vanilla C++20 (`std::mutex` held for a
  copy and nothing else, or `std::atomic`), carrying plain structs - the Qt side converts at
  its edge: senses published to the panel, intent read by the tick. A tick that
  finds no new intent repeats the last - the panel's silence, like the network's, is a repeat
  and then a brake.

Tested deviceless: the mailbox's ordering and the repeat rule, without a window.

## Etape 5 — the first life on the Grid

Master Control with `--disk` and `--log`, one `--window` to watch, one `--program rc_worm` host,
and the User drives the worm across the terraces, into a riser, past the guest, calling - the
first Disk of a life somebody lived. Clu re-simulates the log and agrees. What that life shows
wrong in the Grid goes back to the Grid's repositories as issues, which is the purpose: a worm
whose every action a human chose is the creature a world is debugged with.

## Etape 6 — CI with Qt

The build matrix (MinGW on Windows, GCC on Linux) with Qt fetched by a script of this
repository's own, because the organisation runs GitHub-owned actions only - no third-party
installer action. Cached between runs. The deviceless tests run there; the Grid-in-the-loop
proof of Etape 2 stays a local, recorded check until the flagship publishes release binaries a
workflow may download.

## Later, with triggers

- **The thinking worm** is another repository (`brain-elegans`), wearing this body - trigger:
  the first life on the Grid recorded and its lessons banked.
- **MechaQuokka** as a second body for this same RC Program - trigger: the body format being
  worth a second instance.
- **The Blender body pipeline** (a modelled worm exported to the Grid's model) - trigger: the
  hand-written mesh being too ugly to bear, judged in the window.
