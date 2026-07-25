#!/usr/bin/env python3
"""
Convert a PNG image to a C PROGMEM array of RGB565 uint16_t values.

Usage:
    python3 png_to_rgb565.py block.png                    # -> block.h
    python3 png_to_rgb565.py block.png -n dirt_tile        # custom name
    python3 png_to_rgb565.py block.png --bg 135,206,235    # sky-blue bg for transparent pixels
"""

import argparse
import os
import sys

from PIL import Image


def rgb888_to_rgb565(r, g, b):
    """Pack 8-bit RGB channels into 16-bit RGB565."""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5


def convert(input_path, array_name, output_path, bg_color):
    img = Image.open(input_path)
    w, h = img.size

    pixels = []
    for y in range(h):
        for x in range(w):
            px = img.getpixel((x, y))
            if img.mode in ("RGBA", "LA") or (
                img.mode == "P" and "transparency" in img.info
            ):
                if img.mode in ("RGBA", "LA"):
                    r, g, b, a = px if len(px) == 4 else (*px, 255)
                else:
                    # palette with transparency
                    r, g, b = px[:3]
                    a = 255  # simplified; P mode is rare for game art
                if a < 128:
                    r, g, b = bg_color
            elif img.mode == "RGB":
                r, g, b = px
            elif img.mode == "L":
                r = g = b = px
            elif img.mode == "P":
                r, g, b = img.getpalette()[px * 3 : px * 3 + 3]
            else:
                r, g, b = px[:3]

            rgb565 = rgb888_to_rgb565(r, g, b)
            rgb565 = ((rgb565 & 0xFF) << 8) | ((rgb565 >> 8) & 0xFF)  # swap bytes for ESP32
            pixels.append(rgb565)

    hex_lines = []
    per_line = 16
    for i in range(0, len(pixels), per_line):
        chunk = pixels[i : i + per_line]
        hex_str = ", ".join(f"0x{v:04X}" for v in chunk)
        if i + per_line < len(pixels):
            hex_str += ","
        hex_lines.append(f"    {hex_str}")

    header = (
        f"// {w}x{h} RGB565 tile ({os.path.basename(input_path)})\n"
        f"static const uint16_t {array_name}[{w} * {h}] PROGMEM = {{\n"
        + "\n".join(hex_lines)
        + "\n};\n"
    )

    with open(output_path, "w") as f:
        f.write(header)

    print(f"Wrote {output_path}  ({w}x{h}, {len(pixels)} pixels, {len(pixels)*2} bytes flash)")


def main():
    parser = argparse.ArgumentParser(
        description="Convert a PNG to a C RGB565 PROGMEM array"
    )
    parser.add_argument("file", help="Path to input PNG file")
    parser.add_argument(
        "--name", "-n",
        help="C array variable name (default: basename without extension)",
    )
    parser.add_argument(
        "--output", "-o",
        help="Output .h file path (default: <name>.h alongside input)",
    )
    parser.add_argument(
        "--bg",
        help="R,G,B color for transparent pixels (default: 0,0,0 = black)",
        default="0,0,0",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.file):
        sys.exit(f"Error: file not found: {args.file}")

    try:
        bg = tuple(int(c.strip()) for c in args.bg.split(","))
        if len(bg) != 3 or not all(0 <= v <= 255 for v in bg):
            raise ValueError
    except Exception:
        sys.exit(f"Error: --bg must be R,G,B with 0-255 values, got '{args.bg}'")

    name = args.name or os.path.splitext(os.path.basename(args.file))[0]
    name = name.replace(" ", "_").replace("-", "_")

    output = args.output or os.path.join(
        os.path.dirname(args.file) or ".", f"{name}.h"
    )

    convert(args.file, name, output, bg)


if __name__ == "__main__":
    main()
