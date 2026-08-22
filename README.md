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

The furniture: the organisation's settings, lint, governance and CI shape, mirrored from the
flagship as every repository's are. The worm itself is staged in [TODO.md](TODO.md) and arrives
etape by etape - the Program that loads, the body it brings, the panel the User steers from,
and its first life on the Grid, recorded to a Disk.

## The Doctrine

- **A Program perceives only what its body senses.** No scene graph, no world state, no side
  door: the senses the ABI hands over are the whole of what the worm knows, and the User at the
  panel sees exactly that and nothing more.
- **The body is the Program's own.** `program_rez` brings the worm's shape to the Grid, and the
  Grid validates it whole or refuses it by name.
- **Remote-controlled first.** The User is the brain. A worm whose every action has a human
  behind it is the creature a world is debugged with; a thinking one comes after, in its own
  repository, wearing this body.
- **Cross-toolchain, deliberately.** The Grid is built with MSVC; this Program builds on every
  compiler the flagship supports and every Qt kit on the owner's machine - MSVC, clang-cl,
  MinGW, LLVM-MinGW, GCC and Clang - so the C ABI between a Program and the Grid is proven
  across compilers rather than assumed within one.

## Building

Nothing to build yet. The first code brings the CMake presets (Windows and Linux, the
flagship's compilers, Qt 6.11 per kit), the vendored Program ABI header, and the panel.

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
