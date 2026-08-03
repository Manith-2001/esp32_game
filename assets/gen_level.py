#!/usr/bin/env python3
import argparse
import random

# Screen / tile constants (matching config.h)
TILE_SIZE = 16
SCREEN_W = 320
SCREEN_H = 240
SCREEN_COLS = SCREEN_W // TILE_SIZE   # 20
SCREEN_ROWS = SCREEN_H // TILE_SIZE   # 15

# Tile indices
AIR   = 0
DIRT  = 1
GRASS = 2
CLOUD = 3

TILE_GLYPH = {
    AIR:   "  ",
    DIRT:  "##",
    GRASS: "^^",
    CLOUD: "~~",
}


def generate_map(seed=None, width=SCREEN_COLS, height=SCREEN_ROWS):
    if seed is not None:
        random.seed(seed)

    grid = [[AIR for _ in range(width)] for _ in range(height)]

    # Bottom 2 rows = solid dirt ground
    grid[height - 1] = [DIRT for _ in range(width)]
    grid[height - 2] = [DIRT for _ in range(width)]

    # Carve random pits in the ground
    num_pits = random.randint(1, max(3, width // 8))
    for _ in range(num_pits):
        pit_w = random.randint(2, 4)
        pit_x = random.randint(0, width - pit_w)
        for col in range(pit_x, pit_x + pit_w):
            grid[height - 2][col] = AIR
            grid[height - 1][col] = AIR

    # Scatter grass on surface dirt blocks (dirt with air above)
    for col in range(width):
        for row in range(height):
            if grid[row][col] == DIRT and row > 0 and grid[row - 1][col] == AIR:
                if random.random() < 0.5:
                    grid[row][col] = GRASS

    # Scatter floating platforms (dirt only, no grass)
    num_platforms = max(2, width // 8)
    for _ in range(num_platforms):
        plat_w = random.randint(2, 4)
        plat_x = random.randint(0, width - plat_w)
        plat_y = random.randint(3, height - 5)
        for col in range(plat_x, plat_x + plat_w):
            grid[plat_y][col] = DIRT
            grid[plat_y + 1][col] = DIRT

    # Scatter single cloud tiles in the sky
    for col in range(width):
        for row in range(1, 8):
            if grid[row][col] == AIR and random.random() < 0.08:
                grid[row][col] = CLOUD

    # Safe spawn: guarantee solid ground at cols 0..2 so player doesn't drop
    # into a pit immediately. Fill the bottom two rows solid span across spawn.
    for col in range(0, min(3, width)):
        grid[height - 1][col] = DIRT
        grid[height - 2][col] = DIRT

    return grid


def pretty_print(grid):
    for row in grid:
        line = "".join(TILE_GLYPH[cell] for cell in row)
        print(f"|{line}|")


def emit_c_array(grid, name="level_map"):
    rows = len(grid)
    cols = len(grid[0]) if rows else 0
    print(f"// {cols}x{rows} level map")
    print(f"static const uint8_t {name}[{rows}][{cols}] PROGMEM = {{")
    for row in grid:
        inner = ", ".join(str(c) for c in row)
        print(f"    {{ {inner} }},")
    print("};")


def main():
    parser = argparse.ArgumentParser(
        description="Generate a randomized level map (tiles at %dpx each)" % TILE_SIZE
    )
    parser.add_argument("--seed", "-s", type=int, default=None,
                        help="Random seed for reproducible output")
    parser.add_argument("--width", "-w", type=int, default=SCREEN_COLS,
                        help="Level width in tiles (default: %d = screen width)"
                             % SCREEN_COLS)
    parser.add_argument("--height", default=SCREEN_ROWS, type=int,
                        help="Level height in tiles (default: %d = screen height)"
                             % SCREEN_ROWS)
    parser.add_argument("--c", action="store_true",
                        help="Output as C PROGMEM array instead of pretty-print")
    parser.add_argument("--name", "-n", default="level_map",
                        help="C array variable name (default: level_map)")
    args = parser.parse_args()

    grid = generate_map(args.seed, args.width, args.height)

    if args.c:
        emit_c_array(grid, args.name)
    else:
        rows = len(grid)
        cols = len(grid[0]) if rows else 0
        print(f"Level map ({cols}x{rows}, seed={args.seed or 'random'}):")
        pretty_print(grid)


if __name__ == "__main__":
    main()
