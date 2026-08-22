# Tools

Scripts the build and CI run; plain Python 3, no packages.

- `check_abi_vendor.py check [--flagship PATH]` — the vendored Program ABI header matches the
  fingerprint beside it (CI runs this), and optionally byte for byte the flagship checkout at
  `PATH`. The header is the flagship's to move: re-vendor, never edit here.
