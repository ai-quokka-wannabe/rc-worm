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
