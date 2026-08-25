# Changelog

All notable changes to rc-worm are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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
