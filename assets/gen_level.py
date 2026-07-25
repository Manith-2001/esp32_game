#!/usr/bin/env python3
import argparse
import random

# Screen / tile constants (matching config.h)
TILE_SIZE = 16
SCREEN_W = 320
SCREEN_H = 240
COLS = SCREEN_W // TILE_SIZE   # 20
ROWS = SCREEN_H // TILE_SIZE   # 15

# Tile indices
AIR   = 0
DIRT  = 1
GRASS = 2

TILE_GLYPH = {
    AIR:   "  ",
    DIRT:  "##",
    GRASS: "^^",
}


def generate_map(seed=None):
    if seed is not None:
        random.seed(seed)

    grid = [[AIR for _ in range(COLS)] for _ in range(ROWS)]

    # Bottom 2 rows = solid ground, top ground row = grass
    grid[ROWS - 1] = [DIRT for _ in range(COLS)]
    grid[ROWS - 2] = [GRASS for _ in range(COLS)]

    # Carve random pits in the ground
    num_pits = random.randint(1, 3)
    for _ in range(num_pits):
        pit_w = random.randint(2, 4)
        pit_x = random.randint(0, COLS - pit_w)
        for col in range(pit_x, pit_x + pit_w):
            grid[ROWS - 2][col] = AIR
            grid[ROWS - 1][col] = AIR

    # Scatter floating platforms
    num_platforms = random.randint(2, 4)
    for _ in range(num_platforms):
        plat_w = random.randint(2, 4)
        plat_x = random.randint(0, COLS - plat_w)
        plat_y = random.randint(3, 10)
        for col in range(plat_x, plat_x + plat_w):
            grid[plat_y][col] = GRASS
            grid[plat_y + 1][col] = DIRT

    return grid


def pretty_print(grid):
    for row in grid:
        line = "".join(TILE_GLYPH[cell] for cell in row)
        print(f"|{line}|")


def emit_c_array(grid, name="level_map"):
    print(f"// {COLS}x{ROWS} level map")
    print(f"static const uint8_t {name}[{ROWS}][{COLS}] PROGMEM = {{")
    for row in grid:
        inner = ", ".join(str(c) for c in row)
        print(f"    {{ {inner} }},")
    print("};")


def main():
    parser = argparse.ArgumentParser(
        description="Generate a randomized level map (20x15 tiles, 16px each)"
    )
    parser.add_argument("--seed", "-s", type=int, default=None,
                        help="Random seed for reproducible output")
    parser.add_argument("--c", action="store_true",
                        help="Output as C PROGMEM array instead of pretty-print")
    parser.add_argument("--name", "-n", default="level_map",
                        help="C array variable name (default: level_map)")
    args = parser.parse_args()

    grid = generate_map(args.seed)

    if args.c:
        emit_c_array(grid, args.name)
    else:
        print(f"Level map ({COLS}x{ROWS}, seed={args.seed or 'random'}):")
        pretty_print(grid)


if __name__ == "__main__":
    main()
