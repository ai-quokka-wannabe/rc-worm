#!/usr/bin/env python3
"""
Judge a deployed Program directory: every DLL the worm imports - and every DLL those import, all
the way down - is found beside it or is Windows' own. Nothing may resolve from a Qt kit's
directory, a build tree or the PATH of whoever ran the install, because the Grid's process on
another machine has none of those.

    python tools/check_deploy.py build/windows-msvc-deploy/programs

Exit status 0 with the closed set of libraries listed; 1 naming the first import that is
neither beside the worm nor the system's. Standard library only; reads import tables through
tools/dll_imports.py. The directory is taken from the operator and required to sit inside the
working directory, as every path this repository's tools take is.
"""

import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dll_imports  # noqa: E402

# Windows' own, by name or by family. The Universal C Runtime and the Visual C++ runtime are
# here because `windeployqt --compiler-runtime` ships the redistributable installer beside the
# Program rather than the DLLs, and the Grid - built with the same MSVC - has them installed.
SYSTEM_EXACT = {
    "kernel32.dll", "user32.dll", "gdi32.dll", "advapi32.dll", "shell32.dll", "ole32.dll",
    "oleaut32.dll", "shlwapi.dll", "ws2_32.dll", "winmm.dll", "comdlg32.dll", "imm32.dll",
    "version.dll", "netapi32.dll", "userenv.dll", "wtsapi32.dll", "dwmapi.dll", "uxtheme.dll",
    "setupapi.dll", "d3d11.dll", "d3d12.dll", "dxgi.dll", "dwrite.dll", "d2d1.dll",
    "authz.dll", "crypt32.dll", "bcrypt.dll", "ncrypt.dll", "mpr.dll", "rpcrt4.dll",
    "secur32.dll", "shcore.dll", "comctl32.dll", "gdiplus.dll", "msvcrt.dll", "ucrtbase.dll",
    "msvcp140.dll", "msvcp140_1.dll", "msvcp140_2.dll", "vcruntime140.dll", "vcruntime140_1.dll",
    "concrt140.dll", "ntdll.dll", "kernelbase.dll", "wininet.dll", "winhttp.dll", "iphlpapi.dll",
    "dnsapi.dll", "dbghelp.dll", "psapi.dll", "propsys.dll", "cfgmgr32.dll", "powrprof.dll",
    "synchronization.dll", "d3dcompiler_47.dll", "opengl32.dll", "glu32.dll",
    # Windows' own ICU (Windows 10 1903 and later): the MSVC Qt kits link it rather than ship one.
    "icuuc.dll", "icuin.dll", "icu.dll",
    # What the platform plugin and the styles draw with: DirectX, DirectComposition, the imaging
    # codecs, the print spooler's stub and the schannel TLS that windeployqt insists on shipping.
    "d3d9.dll", "dcomp.dll", "windowscodecs.dll", "winspool.drv", "msimg32.dll", "wintrust.dll",
    "usp10.dll", "oleacc.dll", "uiautomationcore.dll", "shell32.dll", "winmm.dll", "hid.dll",
    "dinput8.dll", "xinput1_4.dll", "avrt.dll", "mfplat.dll", "mf.dll", "dxva2.dll", "evr.dll",
    "kernel32.dll", "cryptui.dll", "schannel.dll", "sspicli.dll", "urlmon.dll", "wldap32.dll",
}
PLUGIN_DIRECTORIES = ("platforms", "styles", "imageformats", "iconengines", "generic", "tls", "networkinformation")
SYSTEM_PREFIXES = ("api-ms-win-", "ext-ms-win-")


def is_system(name: str) -> bool:
    lower = name.lower()
    return lower in SYSTEM_EXACT or lower.startswith(SYSTEM_PREFIXES)


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: check_deploy.py <programs directory>", file=sys.stderr)
        return 1
    root = os.path.realpath(os.getcwd())
    full = os.path.realpath(os.path.normpath(os.path.join(root, argv[1])))
    if not full.startswith(root + os.sep):
        print(f"{argv[1]}: refusing a directory outside the working directory {root}", file=sys.stderr)
        return 1
    directory = Path(full)
    if not directory.is_dir():
        print(f"{argv[1]}: not a directory", file=sys.stderr)
        return 1

    beside = {entry.name.lower(): entry for entry in directory.iterdir() if entry.is_file()}
    worm = beside.get("rc_worm.dll")
    if worm is None:
        print(f"{argv[1]}: no rc_worm.dll beside which to judge", file=sys.stderr)
        return 1

    # Breadth first from the worm and from every Qt plugin windeployqt placed under it: a plugin
    # is loaded by Qt later, and what it imports must be beside the worm or the system's just
    # the same. Each library reached is opened and its imports judged too.
    closed: dict[str, list[str]] = {}
    queue = [worm]
    for plugin_directory in PLUGIN_DIRECTORIES:
        folder = directory / plugin_directory
        if folder.is_dir():
            queue.extend(sorted(entry for entry in folder.iterdir() if entry.suffix.lower() == ".dll"))
    while queue:
        library = queue.pop(0)
        key = library.name.lower() if library.parent == directory else f"{library.parent.name}/{library.name.lower()}"
        if key in closed:
            continue
        try:
            names = dll_imports.imports(library)
        except (OSError, ValueError) as error:
            print(f"{library.name}: cannot read its imports: {error}", file=sys.stderr)
            return 1
        closed[key] = names
        for name in names:
            lower = name.lower()
            if is_system(name):
                continue
            found = beside.get(lower)
            if found is None:
                print(f"{library.name} imports {name}, which is neither beside it in {argv[1]} nor Windows' own.", file=sys.stderr)
                return 1
            queue.append(found)

    print(f"{len(closed)} librar{'y' if len(closed) == 1 else 'ies'} form a closed set beside rc_worm.dll:")
    for name in sorted(closed):
        shipped = [n for n in closed[name] if not is_system(n)]
        print(f"  {name}  ->  {', '.join(shipped) if shipped else '(the system only)'}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
