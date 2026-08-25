#!/usr/bin/env python3
import sys
import struct
from pathlib import Path

def main() -> int:
    if len(sys.argv) != 4:
        print("usage: SPVToHeader.py <input.spv> <output.h> <symbol_name>", file=sys.stderr)
        return 2

    in_path = Path(sys.argv[1])
    out_path = Path(sys.argv[2])
    symbol = sys.argv[3]

    if not in_path.exists():
        print(f"error: input file not found: {in_path}", file=sys.stderr)
        return 3

    data = in_path.read_bytes()
    if len(data) == 0:
        print(f"error: empty input: {in_path}", file=sys.stderr)
        return 4

    if (len(data) % 4) != 0:
        print(f"error: SPIR-V size must be multiple of 4 bytes (got {len(data)}): {in_path}", file=sys.stderr)
        return 5

    word_count = len(data) // 4
    words = struct.unpack("<{}I".format(word_count), data)

    out_path.parent.mkdir(parents=True, exist_ok=True)

    with out_path.open("w", encoding="utf-8", newline="\n") as f:
        f.write("#pragma once\n")
        f.write("#include <cstdint>\n")
        f.write("#include <cstddef>\n\n")
        f.write(f"alignas(4) inline constexpr std::uint32_t {symbol}[] = {{\n")

        per_line = 8
        for i, w in enumerate(words):
            if i % per_line == 0:
                f.write("  ")
            f.write(f"0x{w:08x}u")
            if i != (len(words) - 1):
                f.write(", ")
            if (i % per_line) == (per_line - 1):
                f.write("\n")

        if (len(words) % per_line) != 0:
            f.write("\n")

        f.write("};\n")
        f.write(f"inline constexpr std::size_t {symbol}_SIZE = sizeof({symbol});\n")

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
