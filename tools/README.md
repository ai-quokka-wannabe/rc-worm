# Tools

Scripts the build and CI run; plain Python 3, no packages.

- `check_abi_vendor.py check [--flagship PATH]` — the vendored Program ABI header matches the
  fingerprint beside it (CI runs this), and optionally byte for byte the flagship checkout at
  `PATH`. The header is the flagship's to move: re-vendor, never edit here.

## gait_figure

`gait_figure/` is the one tool in C++: a small executable that ticks the worm's real gait
(`WormLib::Gait`, linked from the `worm` library - never a second model of it) through eight
scripted seconds of rest, full forward, forward with a full left turn and letting go, and writes
[`docs/figures/gait.svg`](../docs/figures/gait.svg) by hand. Coordinates are rounded to a tenth
of a pixel, so a rerun on the same build writes the same bytes. It is built with everything else
and run once by `ctest` (into the build tree) so it cannot rot, and it is never installed: it is
no part of the Program.

After a change to the gait, regenerate the figure from the repository root and commit it with the
change:

```text
cmake --workflow --preset windows-msvc
build/windows-msvc/bin/Release/gait_figure docs/figures/gait.svg
```

Any preset works the same way: `build/<preset>/bin/<config>/gait_figure docs/figures/gait.svg`.
