# Development Environment Setup

How to build, test and deploy rc-worm - the first Program of the Grid - from nothing, on Windows
or Linux, with or without its Qt panel, exactly as CI does; and how to put the worm where the
Grid will find it. To run the whole ecosystem around it, see the flagship's
[RUNNING_THE_GRID.md](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/RUNNING_THE_GRID.md).

---

**The short version**, for someone who has done this before:

```text
git clone https://github.com/ai-quokka-wannabe/rc-worm.git
cd rc-worm
cmake --workflow --preset windows-msvc                     # needs Qt 6.11.2 msvc2022_64 at C:\Qt\6.11.2
cmake --install build/windows-msvc --config Release --prefix build/windows-msvc-deploy
python tools/check_deploy.py build/windows-msvc-deploy/programs
```

Then copy `build/windows-msvc-deploy/programs/*` into the Grid's `programs/` directory. Without
Qt at all: `cmake --preset windows-msvc -DRC_WORM_PANEL=OFF` builds the worm with no panel.

---

## Prerequisites

| Tool | Version | Where to get it |
|------|---------|-----------------|
| CMake | 3.25 or newer | <https://cmake.org/download/> |
| Ninja | any recent | Bundled with Visual Studio; `C:\Qt\Tools\ninja` from the Qt installer; `ninja-build` on Linux |
| A C++20 compiler | see the table below | see the table below |
| Qt | **6.11.2**, Widgets - the version CI pins and the presets name; the kit must match the compiler | The Qt online installer (<https://www.qt.io/download-open-source>) or aqtinstall (below) |
| Python | 3.10 or newer, for the `tools/` scripts and the pinned formatter | <https://www.python.org/downloads/> |
| Node.js | 20 or newer, only for the markdown linter (`npm ci`) | <https://nodejs.org/> |

Qt is used in exactly one place - the panel, `src/panel/` - and **the worm itself needs none of
it**: the body, the seam and the ABI boundary are vanilla C++20 and the standard library.
`-DRC_WORM_PANEL=OFF` builds and tests everything but the panel with no Qt installed, which is
what the "panel off" legs of CI do on every compiler.

There is no Link submodule here and no Rust: the worm sits below the wire. What it vendors is the
flagship's Program ABI header, `libs/program-abi/`, byte for byte (see "Re-vendoring the ABI").

### Compilers, and the Qt kit each one needs

| Platform | Compiler | Configure preset | Qt kit the preset names | Where the compiler comes from |
|----------|----------|------------------|-------------------------|-------------------------------|
| Windows | MSVC (Visual Studio 2022 or newer) | `windows-msvc` | `C:\Qt\6.11.2\msvc2022_64` | The "Desktop development with C++" workload |
| Windows | Clang-CL | `windows-clang-cl` | `C:\Qt\6.11.2\msvc2022_64` (the MSVC kit: clang-cl is MSVC-ABI) | The workload's "C++ Clang tools for Windows", or <https://releases.llvm.org/> |
| Windows | MinGW-w64 GCC 13.1 | `windows-mingw` | `C:\Qt\6.11.2\mingw_64` | Qt's Tools: `C:\Qt\Tools\mingw1310_64` |
| Windows | LLVM-MinGW 17 (or 22) | `windows-llvm-mingw` | `C:\Qt\6.11.2\llvm-mingw_64` | Qt's Tools: `C:\Qt\Tools\llvm-mingw1706_64` (the kit's own compiler) or `llvm-mingw2217_64` |
| Linux | GCC 12+ | `linux-gcc` | the kit on `CMAKE_PREFIX_PATH` (below) | `build-essential` |
| Linux | Clang 15+ | `linux-clang` (+ `-asan`, `-tsan`) | the same | `clang` |

The presets name the Windows kits by their default installer paths; a kit elsewhere is passed
as `-DCMAKE_PREFIX_PATH=<kit>` at configure time, which is also how Linux names its kit. A kit
and a compiler must match: MinGW code cannot link MSVC's Qt and vice versa.

---

## Windows

### Step 1 - Qt

Run the Qt online installer, choose **Qt 6.11.2**, and tick the kits for the compilers you will
build with - `MSVC 2022 64-bit` for `windows-msvc` and `windows-clang-cl`, `MinGW 13.1.0 64-bit`
for `windows-mingw`, `LLVM-MinGW 17.0.6 64-bit` for `windows-llvm-mingw` - plus, under **Tools**,
the matching MinGW toolchains and **Ninja**. The defaults land everything under `C:\Qt`, where the
presets expect it.

Or, without the installer, the way CI does it - aqtinstall, kept out of the system Python with
pipx:

```text
pipx run aqtinstall install-qt --base https://download.qt.io/ --outputdir C:\Qt windows desktop 6.11.2 win64_msvc2022_64
pipx run aqtinstall install-qt --base https://download.qt.io/ --outputdir C:\Qt windows desktop 6.11.2 win64_mingw
pipx run aqtinstall install-qt --base https://download.qt.io/ --outputdir C:\Qt windows desktop 6.11.2 win64_llvm_mingw
```

(`.github/actions/setup-qt/action.yml` is the pinned, checksummed version of this.)

### Step 2 - The compiler's shell

- **MSVC / clang-cl**: a Developer Command Prompt ("x64 Native Tools"), or any shell after
  `vcvars64.bat`.
- **MinGW**: any shell with the toolchain and Ninja first on the `PATH`:
  `set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\ninja;%PATH%`
- **LLVM-MinGW**: `set PATH=C:\Qt\Tools\llvm-mingw1706_64\bin;C:\Qt\Tools\ninja;%PATH%` -
  the 6.11.2 kit is built by clang 17, so 17.0.6 is its match; LLVM-MinGW 22.1.7
  (`llvm-mingw2217_64`, offered beside it under Tools) builds and passes the suite against the
  same kit too, verified 2026-08-29

### Step 3 - Build and test

```text
git clone https://github.com/ai-quokka-wannabe/rc-worm.git
cd rc-worm
cmake --workflow --preset windows-msvc            # configure, build Debug and Release, test both
```

Or step by step: `cmake --preset windows-msvc`, `cmake --build --preset windows-msvc-release`,
`ctest --preset windows-msvc-release`. The library lands in `build/windows-msvc/bin/Release/`.
With the panel built, `vtable_tests` opens real windows - the Qt thread starts in `library_init`
and each rez opens a panel - and `tst_panel` is a Qt Test; both need a desktop session.

### Step 4 - Deploy

```text
cmake --install build/windows-msvc --config Release --prefix build/windows-msvc-deploy
python tools/check_deploy.py build/windows-msvc-deploy/programs
```

The install runs `windeployqt --compiler-runtime` on the DLL and leaves a closed set beside it -
the worm, Qt Core, Gui and Widgets, the platform plugin and the style - and `check_deploy.py`
judges exactly that: every import beside the worm or Windows' own, nothing from the kit's
directory or the build tree. Copy `build/windows-msvc-deploy/programs/*` into the Grid's
`programs/` (beside `TronGridLite.exe`); the Grid loads a Program with its own directory on the
search path, so **no Qt needs to be on the `PATH`**. `TronGridLite --list-programs` then says
`rc_worm - USABLE`.

## Linux (Ubuntu / Debian)

The distribution's Qt 6 (6.4 on Ubuntu 24.04) is older than the 6.11 the panel is written
against; install the kit the way CI does, with aqtinstall:

```text
sudo apt update
sudo apt install -y build-essential clang cmake ninja-build git python3 pipx \
    libgl1-mesa-dev libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 \
    libxcb-render-util0 libxcb-shape0 libxcb-cursor0 libxcb-xinerama0 libfontconfig1 \
    xvfb fonts-dejavu-core
pipx run aqtinstall install-qt --base https://download.qt.io/ --outputdir ~/Qt linux desktop 6.11.2 linux_gcc_64
git clone https://github.com/ai-quokka-wannabe/rc-worm.git
cd rc-worm
cmake --preset linux-gcc -DCMAKE_PREFIX_PATH=~/Qt/6.11.2/gcc_64
cmake --build --preset linux-gcc-release
ctest --preset linux-gcc-release
```

Without a display, run the tests under `xvfb-run -a ctest --preset linux-gcc-release`, as CI
does; without Qt at all, configure with `-DRC_WORM_PANEL=OFF` and skip the kit. The installed
library carries `$ORIGIN` as its RPATH, so a Qt placed beside it is found first and the kit's
`lib` otherwise (put it on `LD_LIBRARY_PATH` for the Grid's process, or copy the kit's libraries
beside the worm).

---

## Testing, exactly as CI does

| Leg | What it does | At home |
|-----|--------------|---------|
| Quick checks | markdownlint; the pinned clang-format gate; `tools/check_abi_vendor.py check` (the vendored ABI header matches its fingerprint); the Qt pin agrees with the presets | `npm ci && npm run lint:md`; the formatter below; the script |
| Build, panel off (five compilers) | Every preset with `-DRC_WORM_PANEL=OFF`, Debug and Release, every ctest | `cmake --workflow --preset <preset>` after `cmake --preset <preset> -DRC_WORM_PANEL=OFF` |
| Build with Qt (four kits) | MSVC, clang-cl, Linux GCC and Clang with the panel; Windows legs install and run `check_deploy.py`; Linux under xvfb; the Linux Clang leg also runs clang-tidy's `concurrency-*` as errors | as above, with the kit; `run-clang-tidy -p build/linux-clang -checks='-*,concurrency-*' -warnings-as-errors='*'` |
| Sanitisers | ASan+UBSan and TSan panel off; ASan+UBSan with Qt under xvfb; **TSan with Qt** on the offscreen platform with the glib dispatcher off, `tools/tsan.supp` naming Qt's own hand-offs | `cmake --workflow --preset linux-clang-asan`; for TSan with Qt: `QT_QPA_PLATFORM=offscreen QT_NO_GLIB=1 TSAN_OPTIONS=suppressions=$PWD/tools/tsan.supp ctest --preset linux-clang-tsan-debug` |
| CodeQL | C/C++, Python, workflows | Read an alert; close it by code, by containment |

The C++ formatter is **clang-format 23.1**, pinned; install exactly that into your user site and
run it on every changed source (`python -m pip install --user "clang-format~=23.1"`, then
`python -m clang_format -i src/...`). Never an LLVM `clang-format` of another major.

---

## Re-vendoring the ABI

`libs/program-abi/` is the flagship's `tgl_program_abi.h` and `abi_fingerprint.txt`, copied
verbatim - this repository never edits them. When the Grid moves its ABI, copy both files from a
tron-grid-lite checkout beside this one and check:

```text
python tools/check_abi_vendor.py check --flagship ../tron-grid-lite
```

The flagship checkout must live under the same parent directory as this one (the tool refuses a
path outside it, in words). Then rebuild: `TronGridLite --list-programs` names, by version, any
Program built against the wrong ABI.

---

## The first life

With the world and the Grid built (see RUNNING_THE_GRID.md), `tools/first_life.ps1` starts
Master Control recording to a Disk and a log, the Grid's window, and the Grid hosting the worm
with its panel, waits for Master Control to end (Ctrl+C in its window) and runs Clu on what was
recorded:

```text
.\tools\first_life.ps1 -Grid ..\tron-grid-lite\build\windows-msvc\src\Release\TronGridLite.exe -MasterControl ..\master-control\target\release\master-control.exe
```

`docs/FIRST_LIFE.md` says what runs, what to do at the keys, what to look for and where a
finding goes; `docs/PANEL.md` explains every panel and the threads behind it; `docs/BODY.md` the
body and the chain.

---

## Troubleshooting

### `Could not find a package configuration file provided by "Qt6"`

The kit the preset names is not there, or it is another compiler's. Install Qt 6.11.2 for that
compiler (the table above), or pass the kit you have: `-DCMAKE_PREFIX_PATH=<kit>`. Or build
without the panel: `-DRC_WORM_PANEL=OFF`.

### The host starts but no panel appears; or the host aborts on start

A Qt without a platform plugin - or, on Linux, without a display - aborts the process, so the
panel refuses to start rather than let that happen. On Windows, deploy with `cmake --install`
and copy the whole `programs/` directory, plugins included; `check_deploy.py` tells you what is
missing. On Linux, set `DISPLAY` (or run under `xvfb-run`) and make the kit's `lib` findable.

### `windeployqt` is not found at install time

The install runs the kit's own `windeployqt`; the kit must be complete (the online installer's
default) and named by `CMAKE_PREFIX_PATH`, which the preset does.

### The vendored ABI is refused by the Grid (`UNUSABLE ... built against ABI version N`)

The Grid moved its ABI. Re-vendor as above and rebuild.

### `check_deploy.py` names an import from `C:\Qt\...`

Something was built against the kit's directory rather than deployed; re-run the install into a
fresh prefix. The closed set is the point: a worm that works only where Qt is installed is a
worm nobody else can run.

### A warning stops the build

`/WX` on MSVC, `-Werror` elsewhere, and clang-tidy's concurrency checks as errors on the Linux
Clang leg. Fix the warning; do not disable the flag.

---

*See `CONTRIBUTING.md` for the pull-request workflow and `README.md` for what lives here.*
