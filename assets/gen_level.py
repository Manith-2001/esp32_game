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

# Generation limits tuned to the player physics in player.h. The player can
# clear roughly two tiles vertically and one empty tile horizontally during a
# rising jump, so platform geometry stays within those limits.
PLAYER_JUMP_HEIGHT_TILES = 2
MAX_PLATFORM_GAP_TILES = 1
MAX_PLATFORM_STEP_ROWS = 1
PLATFORM_MIN_WIDTH = 2
PLATFORM_MAX_WIDTH = 4
PIT_MIN_WIDTH = 2
PIT_MAX_WIDTH = 2
GRASS_CHANCE = 0.5


def _carve_pits(grid, rng):
    height = len(grid)
    width = len(grid[0])
    ground_row = height - 2

    if width <= 3:
        return

    max_pits = max(1, width // 10)
    num_pits = rng.randint(1, max_pits)
    pits = []
    max_pit_width = min(PIT_MAX_WIDTH, width - 3)
    if max_pit_width < PIT_MIN_WIDTH:
        return

    # Keep one solid tile between pits so two random pits cannot become one
    # unjumpable gap.
    attempts = 0
    while len(pits) < num_pits and attempts < num_pits * 10:
        attempts += 1
        pit_width = rng.randint(PIT_MIN_WIDTH, max_pit_width)
        pit_x = rng.randint(3, width - pit_width)
        pit_end = pit_x + pit_width - 1

        overlaps = any(
            not (pit_end + 1 < old_x or old_end + 1 < pit_x)
            for old_x, old_end in pits
        )
        if overlaps:
            continue

        pits.append((pit_x, pit_end))
        for col in range(pit_x, pit_end + 1):
            grid[ground_row][col] = AIR
            grid[ground_row + 1][col] = AIR


def _add_platforms(grid, rng):
    height = len(grid)
    width = len(grid[0])
    ground_row = height - 2

    if width < PLATFORM_MIN_WIDTH or height < 7:
        return

    # The first platform is two rows above the ground. Later platforms may
    # climb or descend one row at a time, forming a reachable route without
    # creating vertically stacked platform blocks.
    lowest_row = ground_row - PLAYER_JUMP_HEIGHT_TILES
    highest_row = max(1, lowest_row - 2)
    if highest_row > lowest_row:
        return

    platform_count = max(2, width // 8)
    min_width = min(PLATFORM_MIN_WIDTH, width)
    max_width = min(PLATFORM_MAX_WIDTH, width)
    previous_x = None
    previous_width = 0
    previous_row = lowest_row

    for index in range(platform_count):
        platform_width = rng.randint(min_width, max_width)

        if index == 0:
            max_start = min(width - platform_width, 8)
            min_start = min(4, max_start)
            platform_x = rng.randint(min_start, max_start)
            platform_row = lowest_row
        else:
            gap = rng.randint(1, MAX_PLATFORM_GAP_TILES)
            platform_x = previous_x + previous_width + gap
            if platform_x + platform_width > width:
                break

            platform_row = previous_row + rng.randint(
                -MAX_PLATFORM_STEP_ROWS, MAX_PLATFORM_STEP_ROWS
            )
            platform_row = max(highest_row, min(lowest_row, platform_row))

        # Leave air below every platform. This prevents adjacent platform
        # pieces from turning into the old multi-row structures.
        if any(
            grid[platform_row][col] != AIR
            or grid[platform_row + 1][col] != AIR
            for col in range(platform_x, platform_x + platform_width)
        ):
            continue

        for col in range(platform_x, platform_x + platform_width):
            grid[platform_row][col] = DIRT

        previous_x = platform_x
        previous_width = platform_width
        previous_row = platform_row


def _add_grass(grid, rng):
    width = len(grid[0])
    ground_row = len(grid) - 2

    # Grass belongs to the ground surface only. Floating platforms stay dirt,
    # even when their top edge is exposed to air.
    for col in range(width):
        if (grid[ground_row][col] == DIRT
                and grid[ground_row - 1][col] == AIR
                and rng.random() < GRASS_CHANCE):
            grid[ground_row][col] = GRASS


def _add_clouds(grid, rng):
    height = len(grid)
    width = len(grid[0])

    for col in range(width):
        for row in range(1, min(8, height)):
            if grid[row][col] == AIR and rng.random() < 0.08:
                grid[row][col] = CLOUD


def generate_map(seed=None, width=SCREEN_COLS, height=SCREEN_ROWS):
    if width < 1 or height < 3:
        raise ValueError("level dimensions must be at least 1x3")

    rng = random.Random(seed)

    grid = [[AIR for _ in range(width)] for _ in range(height)]

    # Bottom 2 rows = solid dirt ground
    grid[height - 1] = [DIRT for _ in range(width)]
    grid[height - 2] = [DIRT for _ in range(width)]

    _carve_pits(grid, rng)
    _add_platforms(grid, rng)

    # Safe spawn: guarantee solid ground at cols 0..2 so player doesn't drop
    # into a pit immediately. Fill the bottom two rows solid span across spawn.
    for col in range(0, min(3, width)):
        grid[height - 1][col] = DIRT
        grid[height - 2][col] = DIRT

    _add_grass(grid, rng)
    _add_clouds(grid, rng)

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
        seed_label = args.seed if args.seed is not None else "random"
        print(f"Level map ({cols}x{rows}, seed={seed_label}):")
        pretty_print(grid)


if __name__ == "__main__":
    main()
