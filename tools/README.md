# Tools

Scripts the build, CI and the desk run; the Python ones are plain Python 3, no packages.

- `check_abi_vendor.py check [--flagship PATH]` - the vendored Program ABI header matches the
  fingerprint beside it (CI runs this), and optionally byte for byte the flagship checkout at
  `PATH`. The header is the flagship's to move: re-vendor, never edit here.
- `check_deploy.py <programs directory>` - a deployed worm's imports, and its Qt plugins', form a
  closed set beside `rc_worm.dll` or are Windows' own (CI runs this on every Windows Qt leg;
  [docs/PANEL.md](../docs/PANEL.md#deployment)).
- `dll_imports.py <file>` - the DLLs a Windows PE file imports, read from its import table, as
  `dumpbin /dependents` would print them; `check_deploy.py` reads through it.
- `first_life.ps1 -Grid <TronGridLite.exe> -MasterControl <master-control.exe>` - Master Control
  recording, the Grid's window and the Grid hosting the worm with its panel, then Clu
  ([docs/FIRST_LIFE.md](../docs/FIRST_LIFE.md)).
- `tsan.supp` - the ThreadSanitizer suppressions of the "TSan with Qt" leg: Qt's own libraries and
  nothing else.

## gait_figure

`gait_figure/` is the one tool in C++: a small executable that ticks the worm's real gait
(`WormLib::Gait`, linked from the `worm` library - never a second model of it) through eight
scripted seconds of rest, full forward, forward with a full left turn and letting go, and writes
[`docs/figures/gait.svg`](../docs/figures/gait.svg) by hand. Coordinates are rounded to a tenth
of a pixel, so a rerun on the same build writes the same bytes. It is built with everything else
and run once by `ctest` (into the build tree) so it cannot rot, and it is never installed: it is
no part of the Program.

After a change to the gait, regenerate the figure and commit it with the change. The tool takes no
arguments and always writes `gait.svg` into the directory it runs in, so run it from `docs/figures`:

```text
cmake --workflow --preset windows-msvc
cd docs/figures
../../build/windows-msvc/bin/Release/gait_figure
```

Any preset works the same way: `build/<preset>/bin/<config>/gait_figure`, run from `docs/figures`.
