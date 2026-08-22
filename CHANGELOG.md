# Changelog

All notable changes to rc-worm are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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
