#!/usr/bin/env python3
"""Refuse a vendored Program ABI header that drifted from the fingerprint beside it.

The header `libs/program-abi/include/tgl/tgl_program_abi.h` and `libs/program-abi/abi_fingerprint.txt`
are copied verbatim from the flagship (tron-grid-lite), which is the ABI's owner. This repository
never edits the header; it re-vendors. The check computes the flagship's own fingerprint - the
declarations with the version line, the comments and the whitespace removed, SHA-256 - and compares
it with the recorded one, exactly as the flagship's `tools/check_abi_version.py` does, so the two
repositories agree about what "the same header" means.

    check                   the vendored header matches the fingerprint beside it. CI runs this.
    check --flagship PATH   also compare both files byte for byte with a flagship checkout (one
                            that lives beside this repository; any other is refused), so a
                            vendor copy that lags the flagship is named before the Grid refuses
                            the Program as stale.

Exit code is 0 when the vendor copy is consistent and 1 when it is not.
"""

import argparse
import hashlib
import io
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
VENDORED = os.path.join("libs", "program-abi")
HEADER = os.path.join(REPO, VENDORED, "include", "tgl", "tgl_program_abi.h")
FINGERPRINT = os.path.join(REPO, VENDORED, "abi_fingerprint.txt")

VERSION_MACRO = "TGL_ABI_VERSION"


def parse_version_define(line):
    """The value of a `#define TGL_ABI_VERSION <n>` line, or None if this is not one."""
    tokens = line.split()
    if tokens[:1] == ["#"]:
        tokens = tokens[1:]
    elif tokens[:1] == ["#define"]:
        tokens = ["define"] + tokens[1:]
    else:
        return None
    if len(tokens) != 3 or tokens[0] != "define" or tokens[1] != VERSION_MACRO:
        return None
    value = tokens[2]
    if value[-1:] in ("u", "U"):
        value = value[:-1]
    return int(value) if value.isdigit() else None


def strip_comments(text):
    """Remove C comments, leaving string and character literals alone - the flagship's own walk."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in ('"', "'"):
            quote = c
            out.append(c)
            i += 1
            while i < n:
                if text[i] == "\\" and i + 1 < n:
                    out.append(text[i : i + 2])
                    i += 2
                    continue
                out.append(text[i])
                if text[i] == quote:
                    i += 1
                    break
                i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            end = text.find("*/", i + 2)
            i = n if end == -1 else end + 2
            out.append(" ")
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            end = text.find("\n", i)
            i = n if end == -1 else end
            out.append(" ")
            continue
        out.append(c)
        i += 1
    return "".join(out)


def read_header(path):
    """The header's declared version, and the fingerprint of the declarations around it."""
    text = io.open(path, encoding="utf-8", newline="").read()
    versions = []
    kept = []
    for line in text.splitlines():
        version = parse_version_define(line)
        if version is None:
            kept.append(line)
        else:
            versions.append(version)
    if len(versions) != 1:
        sys.stderr.write("%s is defined %d times in %s; expected exactly one.\n" % (VERSION_MACRO, len(versions), path))
        raise SystemExit(1)
    body = " ".join(strip_comments("\n".join(kept)).split())
    return versions[0], hashlib.sha256(body.encode("utf-8")).hexdigest()


def read_fingerprint(path):
    version, digest = None, None
    with io.open(path, encoding="utf-8") as handle:
        for line in handle:
            line = line.strip()
            if line.startswith("version="):
                version = int(line[len("version=") :])
            elif line.startswith("sha256="):
                digest = line[len("sha256=") :]
    if version is None or digest is None:
        sys.stderr.write("%s does not carry both a version and a sha256.\n" % path)
        raise SystemExit(1)
    return version, digest


def same_bytes(a, b):
    with open(a, "rb") as first, open(b, "rb") as second:
        return first.read().replace(b"\r\n", b"\n") == second.read().replace(b"\r\n", b"\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--flagship", help="path to a tron-grid-lite checkout to compare the vendor copy against")
    arguments = parser.parse_args()

    header_version, header_digest = read_header(HEADER)
    recorded_version, recorded_digest = read_fingerprint(FINGERPRINT)
    if header_version != recorded_version or header_digest != recorded_digest:
        sys.stderr.write(
            "The vendored tgl_program_abi.h (version %d, %s) does not match abi_fingerprint.txt (version %d, %s).\n"
            "The header is the flagship's to move: re-vendor both files from tron-grid-lite rather than editing here.\n"
            % (header_version, header_digest[:16], recorded_version, recorded_digest[:16])
        )
        return 1

    if arguments.flagship:
        # The flagship checkout is contained: the path means what the operator means by it
        # (relative to where they run this), but the checkout it names must live under the
        # same parent directory as this one - the four repositories sit side by side - and it
        # is resolved to its real place before a byte is read. One that falls outside is
        # refused in words.
        parent = os.path.realpath(os.path.dirname(REPO))
        flagship = os.path.realpath(os.path.normpath(os.path.join(os.getcwd(), arguments.flagship)))
        if not flagship.startswith(parent + os.sep):
            sys.stderr.write("--flagship %s: refusing a checkout outside %s - the flagship lives beside this repository.\n" % (arguments.flagship, parent))
            return 1
        theirs_header = os.path.join(flagship, VENDORED, "include", "tgl", "tgl_program_abi.h")
        theirs_fingerprint = os.path.join(flagship, VENDORED, "abi_fingerprint.txt")
        for ours, theirs in ((HEADER, theirs_header), (FINGERPRINT, theirs_fingerprint)):
            if not os.path.isfile(theirs):
                sys.stderr.write("No flagship file at %s.\n" % theirs)
                return 1
            if not same_bytes(ours, theirs):
                sys.stderr.write("%s differs from the flagship's %s: the vendor copy lags. Re-vendor.\n" % (os.path.relpath(ours, REPO), theirs))
                return 1
        print("The vendored Program ABI is byte for byte the flagship's (version %d)." % header_version)
        return 0

    print("The vendored Program ABI (version %d) matches its fingerprint." % header_version)
    return 0


if __name__ == "__main__":
    sys.exit(main())
