# rc-worm

A remote-controlled glass-neon mecha-worm which plugs into TronGrid Lite as a Program.

The first inhabitant of the Grid. A Program is a shared library the Grid loads through its
Program ABI (`tgl_program_abi.h`): it is handed a body, it receives what that body senses
every tick - its eyes, its ears, its feel of the floor - and it answers with what the body
should do. This Program's brain is a User at a control panel: a Qt window shows the worm what
the worm sees and hears, and the User steers. Remote-controlled, because the first creature
to live in a world should be one whose every action somebody can explain.

## The Four Repositories

rc-worm is one of four repositories in the
[ai-quokka-wannabe](https://github.com/ai-quokka-wannabe) organisation.
[tron-grid-lite](https://github.com/ai-quokka-wannabe/tron-grid-lite) is the Grid - the
renderer, the senses and both client roles, and the home of the Program ABI this repository
builds against; [master-control](https://github.com/ai-quokka-wannabe/master-control) is the
world server every instance of the Grid answers to; [the link repository](https://github.com/ai-quokka-wannabe/link)
is the wire between them. Who owns what, and why every delegation is the way it is, lives in the
flagship's [docs/TOPOLOGY.md](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/TOPOLOGY.md) —
one table, kept in one place, pointed at from everywhere.

## What Lives Here Today

**The Program that loads.** `rc_worm.dll` / `librc_worm.so`: one exported symbol,
`tglGetProgramVTable`, behind which a worm is rezzed, ticked and derezzed exactly as the Program
ABI states - vanilla C++20, `noexcept` at every boundary, standing still for now. It builds on
every compiler the flagship supports with every Qt kit on the owner's machine - and, on the
runners, with Qt fetched by the repository's own composite action (aqtinstall through `pipx`,
pinned; GitHub-owned actions only) - and every one of those libraries loads in the MSVC-built
Grid: `TronGridLite --list-programs` says USABLE at ABI
version 6 for all of them, and a `--program rc_worm` host run against Master Control rezzed the
worm, ticked it two hundred times and left with BYE - a first life, recorded to a Disk, that Clu
re-simulates and agrees with.

**The body.** One rigid segment: a regular icosahedron a quarter metre in circumradius, near-black
mirror faces and a green neon tube along every one of its thirty edges, built from the golden
ratio in vanilla C++20 and lent to the Grid at `program_rez`. It rests on a face with its nose
where the eyes look, passes every rule the Grid and the world judge a model by before either sees
it, stands grounded in Master Control's world, and in the window it is a green-edged icosahedron
mirrored in the floor. [docs/BODY.md](docs/BODY.md) carries the numbers and the why. The panel
and the life somebody lives are staged in [TODO.md](TODO.md).

## The Doctrine

- **A Program perceives only what its body senses.** No scene graph, no world state, no side
  door: the senses the ABI hands over are the whole of what the worm knows, and the User at the
  panel sees exactly that and nothing more.
- **The body is the Program's own.** `program_rez` brings the worm's shape to the Grid, and the
  Grid validates it whole or refuses it by name.
- **Vanilla C++20, Qt only in the panel.** The Program, its body and the seam between the tick
  and the window are standard C++; Qt classes live in the panel module alone, and the worm
  builds and loads without it.
- **Remote-controlled first.** The User is the brain. A worm whose every action has a human
  behind it is the creature a world is debugged with; a thinking one comes after, in its own
  repository, wearing this body.
- **Cross-toolchain, deliberately.** The Grid is built with MSVC; this Program builds on every
  compiler the flagship supports and every Qt kit on the owner's machine - MSVC, clang-cl,
  MinGW, LLVM-MinGW, GCC and Clang - so the C ABI between a Program and the Grid is proven
  across compilers rather than assumed within one.

## Building

CMake 3.25+, Ninja, a C++20 compiler, and - for the panel - Qt 6.11 for the kit of the preset
(`RC_WORM_PANEL=OFF` builds the worm with no Qt at all). The presets are the flagship's:
`windows-msvc`, `windows-clang-cl`, `windows-mingw`, `windows-llvm-mingw`, `linux-gcc`,
`linux-clang`, with `linux-clang-asan` and `linux-clang-tsan` beside them.

```text
cmake --workflow --preset windows-mingw     # configure, build and test, Debug and Release
```

The library lands in `build/<preset>/bin/<config>/`; copy it into the Grid's `programs/`
directory beside `TronGridLite` and run `TronGridLite --program rc_worm`. With the panel built,
the Qt runtime of the kit must be findable by the Grid's process (its `bin` on the PATH) until
Etape 4 settles deployment.

## Licence

Copyright © 2026 Matej Gomboc <https://github.com/ai-quokka-wannabe/rc-worm>.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

See the attached [LICENCE](LICENCE) file for more info.

---

End of line.
