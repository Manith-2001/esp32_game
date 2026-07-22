#!/usr/bin/env python3
import argparse
import os
import sys

def png_to_header(input_path, array_name, output_path):
    with open(input_path, "rb") as f:
        data = f.read()

    hex_lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        hex_str = ", ".join(f"0x{b:02x}" for b in chunk)
        if i + 16 < len(data):
            hex_str += ","
        hex_lines.append(f"  {hex_str}")

    header = f"""// array size is {len(data)}
static const unsigned char {array_name}[] PROGMEM = {{
{chr(10).join(hex_lines)}
}};
"""

    with open(output_path, "w") as f:
        f.write(header)

    print(f"Wrote {output_path} ({len(data)} bytes)")


def main():
    parser = argparse.ArgumentParser(
        description="Convert a PNG file to a C header with raw bytes in PROGMEM"
    )
    parser.add_argument("file", help="Path to input PNG file")
    parser.add_argument(
        "--name", "-n",
        help="C array variable name (default: basename without extension)",
    )
    parser.add_argument(
        "--output", "-o",
        help="Output .h file path (default: <name>.h in current dir)",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.file):
        sys.exit(f"Error: file not found: {args.file}")

    name = args.name or os.path.splitext(os.path.basename(args.file))[0]
    name = name.replace(" ", "_").replace("-", "_")

    output = args.output or os.path.join(os.path.dirname(args.file) or ".", f"{name}.h")

    png_to_header(args.file, name, output)


if __name__ == "__main__":
    main()
