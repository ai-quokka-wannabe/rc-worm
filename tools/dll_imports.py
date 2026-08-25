#!/usr/bin/env python3
"""
List the DLLs a Windows PE file imports, read from its import directory - what
`dumpbin /dependents` prints, without needing the MSVC environment the clang-cl and MinGW builds
do not have. Standard library only.

    python tools/dll_imports.py build/windows-msvc/bin/Release/rc_worm.dll

Exit status 0 with one import per line; 1 with a reason if the file is not a PE image the
script understands. The panel's deployment check (Etape 4) judges this list: everything the
worm imports must be beside it or in the system, never in a Qt kit's directory or the build
tree.
"""

import struct
import sys
from pathlib import Path


def _rva_to_offset(rva: int, sections: list[tuple[int, int, int]]) -> int:
    for virtual_address, virtual_size, raw_pointer in sections:
        if virtual_address <= rva < virtual_address + max(virtual_size, 1):
            return raw_pointer + (rva - virtual_address)
    raise ValueError(f"RVA {rva:#x} lies in no section")


def imports(path: Path) -> list[str]:
    data = path.read_bytes()
    if data[:2] != b"MZ":
        raise ValueError("not a DOS/PE image (no MZ)")
    (pe_offset,) = struct.unpack_from("<I", data, 0x3C)
    if data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise ValueError("no PE signature")
    section_count, optional_size = struct.unpack_from("<HxxxxxxxxxxxxH", data, pe_offset + 6)
    optional_offset = pe_offset + 24
    (magic,) = struct.unpack_from("<H", data, optional_offset)
    if magic == 0x20B:  # PE32+
        directories_offset = optional_offset + 112
    elif magic == 0x10B:  # PE32
        directories_offset = optional_offset + 96
    else:
        raise ValueError(f"unknown optional header magic {magic:#x}")
    import_rva, import_size = struct.unpack_from("<II", data, directories_offset + 8)

    sections = []
    section_offset = optional_offset + optional_size
    for index in range(section_count):
        base = section_offset + (index * 40)
        virtual_size, virtual_address, _raw_size, raw_pointer = struct.unpack_from(
            "<IIII", data, base + 8
        )
        sections.append((virtual_address, virtual_size, raw_pointer))

    names: list[str] = []
    if import_rva == 0 or import_size == 0:
        return names
    descriptor = _rva_to_offset(import_rva, sections)
    while True:
        _lookup, _stamp, _forwarder, name_rva, _thunk = struct.unpack_from(
            "<IIIII", data, descriptor
        )
        if name_rva == 0:
            break
        name_offset = _rva_to_offset(name_rva, sections)
        end = data.index(b"\0", name_offset)
        names.append(data[name_offset:end].decode("ascii", errors="replace"))
        descriptor += 20
    return names


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: dll_imports.py <file.dll|file.exe>", file=sys.stderr)
        return 1
    # An operator's path, judged rather than trusted: resolved, then required to sit inside
    # the working directory - the tool reads build outputs of this checkout and nothing else -
    # and a file rather than a directory or a device.
    root = Path.cwd().resolve()
    path = Path(argv[1]).resolve()
    if root not in path.parents:
        print(f"{argv[1]}: refusing a path outside the working directory {root}", file=sys.stderr)
        return 1
    if not path.is_file():
        print(f"{argv[1]}: not a file", file=sys.stderr)
        return 1
    try:
        names = imports(path)
    except (OSError, ValueError, struct.error) as error:
        print(f"{path}: {error}", file=sys.stderr)
        return 1
    for name in names:
        print(name)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
