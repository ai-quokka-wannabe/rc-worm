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

**Done (2026-08-23).** `src/worm/` (the `Worm` class and the C boundary in `abi.cpp`, the
library `rc_worm`), `src/panel/` (a stub that links `Qt6::Core` and answers `qVersion()` at
`library_init`, so every kit is proven at load; the window is Etape 4), `libs/` adopted from
the flagship (`testing`, `math`, the vendored `program-abi` with `tools/check_abi_vendor.py`
refusing a drifted copy), the flagship's presets plus `windows-llvm-mingw`, each Windows preset
naming its Qt kit. Built and tested on all four Windows kits and with the panel off; every
library USABLE in the MSVC Grid; a 200-tick host run against Master Control with the MinGW
build, recorded, and Clu agreed with every hash. CI builds the panel-off worm on the five
runner compilers plus ASan and TSan; the Qt legs and LLVM-MinGW (not on the runners) are
Etape 6 and local.

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

**Done (2026-08-25).** `src/worm/body.{hpp,cpp}`: the icosahedron from the golden ratio, turned
face-down and nose-forward, a triangular neon prism proud of every edge - 192 vertices, 200
triangles, two materials, lent to the Grid from the body's own storage. `body_tests` holds it to
every rule the Grid and the world judge by (caps, indices, finite floats, area, outward winding,
extent, materials) and to face-down, nose-forward, same-bytes-twice; `vtable_tests` sees the
rez fill the model. Proven on all four Windows kits and panel-off: `--list-programs` USABLE, a
host run in Master Control (192 vertices, 200 triangles, 2 materials; grounded on four contacts
for two hundred ticks; Clu agreed with the Disk), and looked at in `--window`: a green-edged
icosahedron mirrored in the floor, the owner's shape and colour (`docs/BODY.md` carries the
numbers and the why; the dodecahedron stays the recorded alternative).

`program_rez` brings the worm's shape: a glass-neon mecha-worm as a `TglRenderModel` -
vertices, triangles and materials in the Grid's own continuous model (the mirror hull, the neon
of the segment rings; the materials the flagship's MATERIALS.md documents), validated whole by
the Grid and by Master Control (vertex count, triangle indices, extent under four metres, no
subnormal). **One rigid segment** for the first worm, per the flagship's Etape 13 decision: a
body that bends needs a solver able to bend it, and the world's physics is rigid and kinematic.
**The segment is a Platonic solid** (the owner's shape, 2026-08-23): an **icosahedron first** -
twelve vertices and twenty faces that are already triangles, the Grid's own primitive, so no
face is cut and no interior edge is invented; convex, so its hull is itself - with the
dodecahedron (twenty vertices, twelve pentagons cut to thirty-six triangles) as the alternative
if the look in the window wants flatter plates. A near-black mirror for the faces and **green neon tubes along the edges** (the owner's
colour, 2026-08-23 - a visitor's colour beside the Grid's own orange accents), which is where a
glass-neon mecha-worm glows. A chain of them waits for a physics
that can bend.
The Grid decides the senses - the first body's two eyes and two ears, placed by the
`TglCreatureDesc` the Grid hands over - and the worm's nose sits where those eyes are.

Tested without the Grid: the model's arrays are well-formed by the flagship's own rules (every
index inside its array, every float finite and normal, the extent bounded), and the Grid's
`copyValidatedModel` accepts it - run through `--list-programs` and a host run. The body's
extent, mass and sensor stations are written down in a `docs/BODY.md` with the reasoning, so
the later `brain-elegans` can wear the same body knowing what it is.

## Etape 4 — the panel

**Done (2026-08-25).** `src/worm/seam.{hpp,cpp}`: the mailbox, one slot each way, copied whole
under a mutex held for the copy - `SensesSnapshot` (every eye's samples, every ear's band-by-bin
energy and arrivals, every contact, the vestibular numbers; fixed capacities, every drop
COUNTED and shown by name), `BodySnapshot` at rez, `Intent` latest-wins with the call LATCHED
until a tick hears it. `Worm::tick` publishes, takes, and keeps the silence rule: fresh, repeated
for `PANEL_REPEAT_TICKS` (four), then braked; `Applied` names which. `src/panel/`: the Qt thread
(`QApplication` built inside the thread's entry, an anchor for blocking posts; started in
`library_init`, joined in `library_shutdown`; refuses a headless Linux rather than abort) and
`PanelWindow` (eyes on an equirectangular map, ears as band x bin histograms with arrivals,
the feel from above; W/S A/D Space X and sliders scaled to the body's bounds; stylesheet scoped
to the window). Tests: `seam_tests` deviceless, `tst_panel` on Qt Test's thread (draws, offers,
latches, two thousand publishes never wait), `vtable_tests` through the DLL opening real
windows. Deployment: `cmake --install` + `windeployqt --compiler-runtime` on the DLL,
`tools/check_deploy.py` judging a closed set (six libraries on MSVC, nine on MinGW, eight on
LLVM-MinGW), run on every Windows Qt leg; the Grid loads it with no Qt on the PATH since
tron-grid-lite #117 (`LoadLibraryEx` with the library's own directory), and the panel puts its
own directory on Qt's library paths and refuses to start unless a platform plugin is reachable
(Qt would abort the host otherwise). Proven on all five Windows builds, and steered: the
deployed worm hosted in the Grid, `W` three seconds = three metres along -Z, `A` one second =
a right angle, released = braked; Clu agreed with the Disk. `docs/PANEL.md` carries the
threads, the seam, the rule and the why.

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

**Adopted from the owner's `claude-chats-browser`** (a Qt 6 / C++20 desktop app, 2026-08-23):
`qt_standard_project_setup()` and `qt_add_*` for the panel's own targets; Qt Test (`Qt6::Test`,
one `tst_*` executable per unit) for the panel's own tests, beside the flagship's `testing` for
everything vanilla; deployment by `qt_generate_deploy_app_script` / `windeployqt` with
`--compiler-runtime`, pointed at the Program library so the Qt runtime lands beside
`rc_worm.dll` in the Grid's `programs/` directory - the answer to "the Grid's process must find
Qt"; and the offscreen-QPA note for headless tests.

## Etape 5 — the first life on the Grid

**Ready for the User (2026-08-26).** `tools/first_life.ps1` starts Master Control recording,
the Grid's window and the Grid hosting the worm with its panel, waits for Master Control to end
and runs Clu; `docs/FIRST_LIFE.md` is the recipe, what to look for and where what is found
goes. A scripted rehearsal lived first (seven metres in a weave, two calls, the window
watching; Clu agreed, 1600 ticks): the worm hears its own call as the ABI promises, and Master
Control cannot be asked to stop (every Disk ends without an end line - filed against
master-control). The ears now remember their last arrivals for a second, because an arrival is
one tick's event and a human reads slower than the Grid ticks. The life itself is the User's
to live; its Disk and log, kept, are the trigger for the thinking worm.

Master Control with `--disk` and `--log`, one `--window` to watch, one `--program rc_worm` host,
and the User drives the worm across the terraces, into a riser, past the guest, calling - the
first Disk of a life somebody lived. Clu re-simulates the log and agrees. What that life shows
wrong in the Grid goes back to the Grid's repositories as issues, which is the purpose: a worm
whose every action a human chose is the creature a world is debugged with.

## Etape 6 — CI with Qt

**Done (2026-08-25), pulled forward before Etape 4 so the panel is provable the day it lands.**
`.github/actions/setup-qt` (aqtinstall through `pipx run`, pinned to the commit that taught it
Qt 6.11's Windows layout, from `download.qt.io`, every archive checked against Qt's SHA-256;
cache restore-only on pull requests, saved on main; `QTDIR` exported and passed as
`CMAKE_PREFIX_PATH` over the presets' desk paths); `Build with Qt` on windows-msvc,
windows-clang-cl (the MSVC kit), linux-gcc and linux-clang (the GCC kit) with the Linux runtime
packages, `xvfb-run` and a real font; an ASan+UBSan-with-Qt sanitiser leg with
`ASAN_OPTIONS=detect_leaks=0`; a quick-check that `QT_VERSION` in the workflows and the kit
paths in the presets agree; the worm's dynamic dependencies recorded per build for Etape 4's
deployment check to judge. TSan with the panel on waits for the panel's thread (Etape 4).

The Qt legs of the build matrix with Qt fetched by a composite action of this repository's
own - adopted from the owner's `claude-chats-browser`: `.github/actions/setup-qt` runs
`aqtinstall` through `pipx` (a tool, not a third-party action, so the GitHub-owned-actions policy
holds), caches `~/Qt/<version>` restore-only on pull requests and save on main, and installs the
Linux runtime packages (`libgl1-mesa-dev libxkbcommon-x11-0 libxcb-* libfontconfig1 xvfb`). The
MSVC kit on Windows, the GCC kit on Linux; MinGW and LLVM-MinGW kits stay local presets. Cached
between runs. The deviceless tests run there; the Grid-in-the-loop
proof of Etape 2 stays a local, recorded check until the flagship publishes release binaries a
workflow may download.

## Etape 7 — the chain

**Done (2026-08-26), across the four repositories in the order below.** Link #25 (protocol v7),
master-control #32 (the trail along the head's path), tron-grid-lite #118 (ABI v7, a model per
segment pose, skip ranges for the senses), and here: the ABI re-vendored at v7, two joint stubs
authored out of the nose spike and its antipode, `BODY_SEGMENTS` = 8 and `SEGMENT_SPACING` = 0.56 m
declared when the body is lent. `docs/BODY.md` § The chain.

The owner's ruling (2026-08-26): a worm is a chain of icosahedra, joined spike to spike - the
joint sits between two vertices, one of each segment, a short neon between them - and it
undulates as it moves. One rigid segment was Etape 3's deliberate stop, because the world's
physics is rigid and kinematic; the chain needs three things across the repositories, in this
order:

1. **The wire (Link v7)**: a creature carries a segment count and a pose per segment, in
   `TICK_STATE`, the Disk and the log; the REZ carries the segment model once and the joint's
   two spike indices. 0.0.0: no compatibility kept.
2. **The world (Master Control)**: the head is the rigid body it is today; each trailing
   segment is placed one joint length back along the head's recorded path - deterministic,
   allocation-free (a ring of past poses per creature), replayed bit for bit, hashed - so the
   undulation falls out of the path when the User weaves. A lateral wave as a function of
   speed comes after, once the following looks right in the window; it is an authored motion,
   not physics, and says so.
3. **The Grid**: renders the segment model once per segment at its pose, senses stay on the
   head (the eyes and ears are where they are), and the joint neon is drawn between the two
   spikes as a fourth material of the body.
4. **Here**: `docs/BODY.md` names the two joint spikes (a waist vertex fore and aft, so a
   segment's joints are opposite each other), and the panel shows the chain's poses in the
   feel view from above.

## Later, with triggers

- **The thinking worm** is another repository (`brain-elegans`), wearing this body - trigger:
  the first life on the Grid recorded and its lessons banked.
- **MechaQuokka** as a second body for this same RC Program - trigger: the body format being
  worth a second instance.
- **The Blender body pipeline** (a modelled worm exported to the Grid's model) - trigger: the
  hand-written mesh being too ugly to bear, judged in the window.
