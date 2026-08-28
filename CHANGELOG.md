# Changelog

All notable changes to rc-worm are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- `docs/PANEL.md` and `docs/FIRST_LIFE.md` say what a dragging worm now hears: its own spikes on
  the floor, in the ears' earliest bins with fading tails and an arrival count that stays at
  zero (master-control #34 sounds the scrapes, tron-grid-lite #119 delivers them to creature
  ears). What to look for, and what would be a finding, is written down for the first life.

### Fixed

- **The spikes themselves lie on the axis: the body is pitched 10.8°.** The owner's second
  look (2026-08-28): with the stubs bent to meet on the axis the tips met, but the spikes jogged
  9.4 cm at every joint - nose spike 4.7 cm above the axis, tail spike 4.7 cm below. The body is
  now pitched about X by the waist's elevation, so nose and tail vertices lie exactly on -Z and
  +Z, the stubs run straight along the axis, and two neighbours' spikes meet at one point: spike,
  stub, tip, stub, spike, one line. The body no longer lies flat on a face - it stands on the one
  spike the tilt leaves lowest, a sharp spike on the Grid floor as the ruling has it; the world
  stands it by its lowest point as before. Spacing, eyes and ears unchanged. Master Control's
  chain golden is re-recorded against this body.
- **The joint tips lie on the axis, so two segments meet at one point.** The owner's report
  (2026-08-28): the chain's neighbours were not touching. A waist vertex of an icosahedron
  resting on a face sits 0.1876 circumradii off the horizontal axis, so a stub along the spike's
  own direction ended 4.7 cm above the axis at the nose and 4.7 cm below it at the tail, and two
  neighbours' tips stood 10.5 cm apart on a straight chain. The stub now runs from the spike to a
  joint tip exactly on the axis, `JOINT_TIP_REACH` = 0.28 m out (`(0, 0, ∓0.28)`), kinking by
  the waist's 10.8°; `SEGMENT_SPACING` is twice the reach and unchanged at 0.56 m, the posture
  untouched. Master Control's companion places the chain as rigid rods sharing those tips.
- **The feel's text no longer writes through the plan box.** The owner's report (2026-08-28):
  the contact and vestibular rows ran straight under the body-from-above box. A row that shares
  the box's height now stops at its edge with an ellipsis.
- **The feel is no longer born clipped.** Its size hint (340) sat below its own minimum (420),
  so the layout planned a column the minimum then pushed past the window's edge; and the window
  opened at a fixed 1280 x 720 whatever its views asked for. The hint now exceeds the minimum,
  the minimum is low enough to squeeze onto a narrow screen (the rows elide and the chain scales,
  so nothing is lost when the layout presses), and the window opens at what its views ask for,
  bounded by the screen.
- `docs/FIRST_LIFE.md`, `TODO.md` and `tools/first_life.ps1` no longer say Master Control cannot
  be asked to stop: master-control #31 was fixed the same day it was filed, and Ctrl+C in its
  window (or closing it) now ends a life properly - the log ends, the Disk closes, Clu is
  content. The rehearsal's finding stays in the record as what it was.
- `tools/check_abi_vendor.py check --flagship PATH` reads only a checkout that lives beside this
  repository: the path means what it always meant (relative to where the tool is run), but it
  is normalised, resolved to its real place and refused in words unless it falls under the two
  checkouts' common parent - the four repositories sit side by side by doctrine, so nothing is
  lost. This closes CodeQL's two path-injection alerts on the
  operator's path by code rather than by dismissal, in the same form as `tools/dll_imports.py`.

### Added

- **The panel draws its declared chain.** Along the bottom of the feel view runs the chain the
  body's rez lends the Grid - eight icosahedra joined spike to spike over a floor line, in the
  worm's neon, the head brightest with a dot of ink for its eye end - and the header says it:
  `a chain of 8 segment(s), 0.56 m apart`. The declaration travels beside the Grid's body in the
  seam's `BodySnapshot` (`segment_count`, `segment_spacing`, `segment_radius`), filled from the
  body's own constants - one declaration, two readers. It is decorative on purpose: it never
  moves, because where the chain is and how it waves is the world's, heard through the ears,
  never echoed back to the panel.
- **The guides.** The owner's ask (2026-08-27): every repository of the organisation gets a
  development-environment guide a contributor can follow without struggling. Here:
  `docs/DEV_ENV_SETUP.md` - the short version; every compiler with the Qt 6.11.1 kit it needs and
  where both come from (the Qt Tools, or aqtinstall as CI does it); Windows and Linux step by step,
  panel on or off; what CI runs and how to run every leg at home, the TSan-with-Qt leg included;
  deployment into the Grid's `programs/` and the closed set `check_deploy.py` judges; re-vendoring
  the ABI; the first life; troubleshooting. CONTRIBUTING's kit path typo is fixed and both it and
  the README point at the guide and at the flagship's `RUNNING_THE_GRID.md`.
- **clang-tidy's concurrency checks, as errors, in CI.** The `.clang-tidy` the editor runs
  names every family; the Linux Clang build with the panel on now runs `concurrency-*` alone
  over the sources with warnings as errors - the calls that are not thread-safe, which a seam
  between two threads must never make - with Qt's generated code left out. Its first run found
  one: `displayAvailable` read `DISPLAY`, `WAYLAND_DISPLAY` and `QT_QPA_PLATFORM` with
  `std::getenv` on Linux, which is not thread-safe against a host that touches its environment;
  now `qEnvironmentVariableIsSet`, under Qt's own lock.
- **ThreadSanitizer with the panel on.** A fourth sanitiser leg, "TSan with Qt": the seam
  tests and the panel's Qt tests with every access instrumented, on Qt's offscreen platform
  with the glib dispatcher off - the X stack is uninstrumented and built without frame
  pointers, and its mutex use confuses the sanitiser beyond any suppression's reach. Qt is
  not instrumented either, so the hand-offs across its blocking queued invokes (a window
  built and deleted on the Qt thread at the tick thread's request) are made visible by an
  acquire/release atomic on either side in `panel.cpp`, an annotation of what Qt's semaphore
  already guarantees; `tools/tsan.supp` names Qt's own libraries and nothing else. The seam
  is a `std::mutex` the sanitiser sees whole, and a race there is the real thing the leg
  exists to catch. The panel test's one tight timing bound is widened under the sanitiser,
  the measured value still in the message.
- **The chain: eight icosahedra joined spike to spike.** Etape 7, the owner's ruling
  (2026-08-26). The ABI re-vendored at v7; the body authors the joint - a neon stub, the same
  prism the edges wear, out of the nose spike and out of its antipode, half a joint each, so two
  segments meet tip to tip - and declares the chain when it is lent: `BODY_SEGMENTS` = 8,
  `SEGMENT_SPACING` = a diameter plus two stubs = 0.56 m. Where the trailing segments stand is
  Master Control's (a trail along the head's path) and the Grid draws the mesh once per segment;
  the Program is told nothing of its tail. 204 vertices, 212 triangles. `docs/BODY.md` § The
  chain carries the numbers.
- **The first life, ready for the User.** Etape 5's tooling: `tools/first_life.ps1` starts
  Master Control recording to a Disk and a log, the Grid's window and the Grid hosting the worm
  with its panel, waits for Master Control to end and runs Clu; `docs/FIRST_LIFE.md` is the
  recipe, what to look for and where what is found goes. A scripted rehearsal lived first:
  seven metres in a weave with two calls, the window watching, Clu agreeing over 1600 ticks. It
  found the worm hearing its own call exactly as the ABI promises (0.58 ms at both ears, two
  echoes) and Master Control unable to be asked to stop (filed against it). The ears now keep
  their last arrivals for a second, dimmed, with the tick they came in - an arrival is one
  tick's event, and a human reads slower than the Grid ticks. And the owner's ruling for the
  next etape is written down: the chain, icosahedra joined spike to spike, undulating.
- **The panel: the User is the brain.** Etape 4. A Qt Widgets window on the Program's own thread
  shows what the worm senses - each eye's samples where they look on a map of the body frame,
  each ear as the band-by-bin histogram it is with the arrivals' onsets and radial velocities
  marked, every contact where it happened on the body with the vestibular numbers beside - and
  takes the three controls the ABI carries, from keys or sliders scaled to the body's own bounds.
  The seam between the tick and the window is vanilla C++20 (`src/worm/seam.hpp`): one mailbox
  per creature, plain structs copied whole under a mutex held for the copy, senses latest-wins,
  intent latest-wins with a call latched until a tick hears it, fixed capacities with every drop
  counted and shown by name. The tick never waits for a window; a tick that finds no new intent
  repeats the last for four ticks and then brakes. The Qt thread starts in `library_init`, joins
  in `library_shutdown`, opens a window at rez and closes it to completion at derez; a headless
  Linux is refused rather than aborted. Tested deviceless (`seam_tests`), on Qt Test's thread
  (`tst_panel`) and through the DLL with real windows (`vtable_tests`). Deployed by
  `windeployqt --compiler-runtime` at install and judged by `tools/check_deploy.py`, a closed
  set beside `rc_worm.dll` on every Windows kit; the Grid loads it with no Qt on the PATH
  (tron-grid-lite #117). `docs/PANEL.md` carries the threads, the seam, the silence rule and the
  why. Three catches on the way, all found by running the real thing: `qt_add_executable`
  under clang-cl embeds a side-by-side manifest Windows refuses, so the Qt Test target is a
  plain executable with moc on; Qt looks for its platform plugin beside the host executable,
  never beside the DLL, so the panel adds its own directory to Qt's library paths first; and a
  Qt that cannot find that plugin aborts the host, so the panel checks for it before any
  `QApplication` exists and refuses rather than starts. Then the first steered ticks: with the
  deployed worm hosted in the Grid (no Qt on the PATH), `W` held three seconds moved the body
  three metres along its -Z, one second of `A` turned it to exactly a right angle, and it braked
  when the keys were released - recorded to a Disk that Clu re-simulated and agreed with.
- **`/check-coherence`.** A documentation audit for contradictions between clauses that were
  each right when written, orphaned claims about the tree, facts stated twice against the
  single-source-of-truth table, scope drift and stale "today" sections - and one that is willing
  to conclude the documents are coherent. Adopted from the owner's `setonix-os`; the same file
  in every repository of the organisation.
- **CI with Qt.** Etape 6, pulled forward so the panel is provable the day it lands. Adopted
  from the owner's `claude-chats-browser`: `.github/actions/setup-qt`, a composite action of
  this repository's own (GitHub-owned actions only), runs aqtinstall through `pipx run` pinned
  to a commit, fetches the kit from Qt's own server with every archive checked against Qt's
  SHA-256, caches it restore-only on pull requests and saved on main, and exports `QTDIR`,
  which the workflows pass as `CMAKE_PREFIX_PATH` over the presets' desk paths. `Build with Qt`
  runs the MSVC kit on windows-msvc and windows-clang-cl and the GCC kit on linux-gcc and
  linux-clang - Linux tests under `xvfb-run` with a real font, because the offscreen platform
  ships none and lays text out pathologically slowly - and a third sanitiser leg runs ASan+UBSan
  with the panel on, leaks not counted since the un-instrumented Qt stack reports its own. A
  quick-check refuses a tree where the workflows' `QT_VERSION` and the presets' kit paths
  disagree; Dependabot is pointed at the action's directory, whose pins the workflow scan does
  not see. Each Qt build records the worm's dynamic dependencies, for Etape 4's deployment
  check to judge.
- **The body.** `program_rez` brings a regular icosahedron - twelve vertices on a 0.25 m
  circumsphere, twenty faces wound outward, turned to rest on a face with its nose on -Z where
  the first body's eyes look - with a green neon tube along every one of its thirty edges: a
  triangular prism standing a little proud of the shell, 192 vertices and 200 triangles in all,
  a near-black mirror for the shell and green neon at the Grid's own intensities. Built once
  from the golden ratio in vanilla C++20 (`src/worm/body.{hpp,cpp}`) and lent from the body's
  own storage, exactly as the ABI states. `body_tests` holds it to every rule the Grid and the
  world judge a model by before either sees it, plus face-down, nose-forward and same-bytes-
  twice; `docs/BODY.md` carries the numbers and the why. Proven on every Windows kit and
  panel-off: USABLE in the Grid, grounded on four contacts for two hundred ticks in Master
  Control with Clu agreeing, and seen in the window - a green-edged icosahedron mirrored in the
  floor. Two red-first catches on the way: a mis-listed face (the last face of the standard
  icosahedron table wound inward) and a nose turn the wrong way round, both found by the tests
  before the Grid saw the body.
- **The pins Dependabot cannot see are watched weekly.** Adopted from the owner's `arm-dev-env`:
  `tool-updates.yml` reads each pinned tool version out of the tree, resolves the latest
  release from the tool's own feed, and opens one tracking issue per tool that is behind -
  edited on later runs, closed by itself when the pin catches up. An issue, not a pull
  request: a bump is installed on the desk and its checksum re-recorded, a decision rather
  than a merge button.
- **The markdown linter is pinned and every job has a timeout.** Adopted from the owner's
  `arm-cmake-toolchains` and `claude-chats-browser`: `package.json` + `package-lock.json` pin
  markdownlint-cli2 to the byte, `npm ci` installs exactly that, the cache is keyed on the lock
  file, and Dependabot proposes the bumps - a lint run is reproducible and a new linter release
  can no longer redden an unrelated pull request. Every job carries a `timeout-minutes`, so
  nothing can hang for the six-hour default.
- **Formatting is a gate, under a pinned formatter.** Adopted from the owner's
  `claude-chats-browser`: quick-checks installs `clang-format~=22.1` and runs it `--dry-run
  --Werror` over every tracked C and C++ file, so STYLE.md's claim that the tree is
  clang-format clean is enforced rather than hoped for. The desk runs the same major.
- **Every internal link and anchor is checked per pull request, the external ones weekly.**
  Adopted from the owner's `altium-designer-mcp`: `lychee --offline --include-fragments` in
  quick-checks, installed from its pinned release with a checksum rather than through a
  third-party action, so a dead anchor is a red pull request; and `links.yml`, a scheduled
  workflow that follows the external links too, never blocking a merge on a site elsewhere.
- **The Program that loads.** `rc_worm.dll` / `librc_worm.so`, one exported symbol, a worm
  rezzed, ticked and derezzed behind it exactly as the Program ABI states - vanilla C++20,
  `noexcept` throughout, standing still. The flagship's presets adopted (plus
  `windows-llvm-mingw`), each Windows preset naming its Qt 6.11.1 kit; `libs/testing` and
  `libs/math` adopted; the Program ABI vendored with a drift check in CI. The panel is a stub
  that links `Qt6::Core` and answers `qVersion()` at load, so every kit is proven inside the
  host process; `RC_WORM_PANEL=OFF` builds with no Qt. Proven: all four Windows kits and the
  panel-off build pass their tests, every one of those libraries is USABLE in the MSVC Grid,
  and the MinGW-built worm lived 200 ticks in Master Control's world - rezzed, grounded, left
  with BYE - recorded to a Disk that Clu re-simulated and agreed with. One breakage round on
  the drift check (a vendor copy edited) caught.
- **The furniture, and the worm unpaused.** The organisation's settings mirrored from the
  flagship - editor and lint configuration, `.clang-format` and `.clang-tidy`, the governance
  files, issue and pull request templates, Dependabot for the actions, the cache clean-up
  workflow, and the CI shape with markdown quick-checks feeding the `CI Success` gate; the
  build matrix arrives with the first code. `LICENSE` renamed to `LICENCE`, content untouched.
  The README says what the worm is and the doctrine it lives by; `TODO.md` stages the six
  etapes from the Program that loads to CI with Qt; `.claude/CLAUDE.md` carries the rules.
