# rc-worm

A remote-controlled glass-neon mecha-worm which plugs into TronGrid Lite as a Program - the
first inhabitant of the Grid. The User is the brain: a Qt panel shows what the worm senses and
the User steers. The sibling repositories (org `ai-quokka-wannabe`): `tron-grid-lite` - the
Grid, the flagship whose conventions this repo mirrors and whose Program ABI this repo builds
against; `master-control` - the world server (the being is **Master Control**; lower case
names only its repository); `link` - the wire (the library is **Link**).

**Two facts that govern every decision in this repo:**

1. **This repository is a Program. It perceives only what its body senses.** The ABI hands
   over eyes, ears and feel; nothing else exists. No scene graph, no world state, no side
   channel to the Grid or to Master Control, ever - a Program that knew more than its body
   would be the divergence class the organisation was built to exclude. The panel shows the
   User exactly what the worm knows and not one bit more.
2. **The settings are mirrored from the flagship, deliberately.** Repository settings,
   rulesets, CI shape, lint configuration and governance files are copies of
   `tron-grid-lite`'s, kept as identical as the language difference allows - the owner wants
   them identical, not improved. When changing a mirrored setting, change it in the flagship
   too or not at all.

## Rules

- **Language: C++20, CMake 3.25+, Ninja.** Warnings are errors (`/WX`, `-Werror`), as in the
  flagship. `.clang-format` and `.clang-tidy` are the flagship's.
- **Qt lives in the panel and nowhere else** (the owner's ruling, 2026-08-23). Only the GUI
  module (`src/panel/`) may include a Qt header or name a Qt class; the Program, the body, the
  ABI boundary and the thread seam are vanilla C++20 with the standard library alone -
  `std::thread`, `std::atomic`, `std::mutex`, plain structs across the seam. The worm without
  its panel is a worm that builds and loads with no Qt at all, and a CMake option says so.
- **The Program ABI is the flagship's to move.** `tgl_program_abi.h` is vendored verbatim with
  its fingerprint, and a check refuses a vendor copy that drifted from the flagship's. Never
  edit the vendored header here; change it in the flagship and re-vendor.
- **Every compiler the flagship supports, every Qt kit on the owner's machine** (the owner's
  ruling): `windows-msvc`, `windows-clang-cl`, `windows-mingw`, `windows-llvm-mingw`,
  `linux-gcc`, `linux-clang`, plus ASan/TSan on Linux. The Grid is MSVC, so the MinGW and
  LLVM-MinGW builds prove the C ABI across compilers for real; every preset's DLL must load in
  the MSVC Grid alike.
- **`program_tick` never blocks and never throws.** It runs on the Grid's tick thread;
  `noexcept` throughout, a `catch (...)` at every boundary the Grid calls into. The panel lives
  on its own thread and talks through a mailbox each way.
- **A Program touches its state in `program_derez`, never after `library_shutdown`.** The
  flagship's fixtures hold the Grid to this; this Program holds itself to it.
- **Spelling:** British English everywhere. The LICENCE file content is untouchable (legal
  document).
- **Vocabulary:** Tron terms, one word per concept - the Grid, Program, creature, User, Master
  Control, tick, senses, actions. The flagship's STYLE.md § Tron Naming is authoritative.
- **Design authority stays in the flagship.** What a Program is, what it senses, how its body
  is validated: `docs/PROGRAM_INTERFACE.md`, `docs/PERCEPTION.md`, `docs/MATERIALS.md` there.
  Point at them; never copy a table here.
- **Don't over-engineer.** Keep it simple. No abstractions until there's a concrete second use
  case.
- **Licence:** GPL v3-or-later.

## CI today

`quick-checks` (markdown lint, stray carriage returns, the vendored ABI drift check) feeding
`Build (<preset>)` on windows-msvc, windows-clang-cl, windows-mingw, linux-gcc and linux-clang
with the panel off, `Sanitiser (ASan+UBSan)` and `Sanitiser (TSan)`, and the `CI Success` gate
the ruleset requires by its exact name. The Qt legs (Qt fetched by this repository's own
script, GitHub-owned actions only) are Etape 6; LLVM-MinGW is not on the runners and stays a
local preset.

## Process

- **Main is protected: PR + review, direct pushes rejected.** Branch, push, `gh pr create`; the
  owner merges. Signed commits, code-owner review and resolved threads are required - the
  ruleset is a byte-identical copy of the flagship's.
- **Actions policy: GitHub-owned actions only, SHA-pinned.** A single third-party action makes
  the workflow die with `startup_failure` and zero jobs. Never reintroduce one - Qt is fetched
  by script, not by an installer action.
- **Red-first tests, when there is code to test.** Every new check gets broken deliberately once
  before it is trusted, and the break must compile or the round is void.
- **Write commit messages to a scratchpad file and `git commit -F <file>`** - multi-line
  messages through PowerShell mangle quotes.
- The flagship's `.claude/CLAUDE.md` § Hard-won rules applies on this machine wholesale -
  especially: never edit files through PowerShell `Set-Content`/`Out-File`, use the editing
  tools rather than shell heredocs, never run a local clang-format over the tree, and confirm
  the build succeeded before believing a test result.
- **`CHANGELOG.md` § Unreleased gets an entry for anything user-visible**, and `TODO.md`
  records what each etape decided when it lands.
