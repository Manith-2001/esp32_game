#include "debug.h"

#if DEBUG_MODE

#include "player.h"
#include "physics.h"
#include <Arduino.h>

static constexpr uint8_t COLS = Physics::COLS;
static constexpr uint8_t ROWS = Physics::ROWS;
static constexpr uint8_t TILE_S = Physics::TILE_S;

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

static char tile_char(uint8_t t) {
    switch (t) {
        case 0:  return '.';
        case 1:  return 'B';
        case 2:  return 'G';
        case 3:  return 'C';
        default: return '?';
    }
}

static const char *tile_name(uint8_t t) {
    switch (t) {
        case 0:  return "empty";
        case 1:  return "BLOCK";
        case 2:  return "GRASS";
        case 3:  return "CLOUD";
        default: return "???";
    }
}

static uint8_t read_tile(const uint8_t (*level_map)[COLS], int col, int row) {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) {
        if (row >= ROWS) return 0xFE;
        return 0xFF;
    }
    return pgm_read_byte(&level_map[row][col]);
}

static char tile_char_at(const uint8_t (*level_map)[COLS], int px, int py) {
    if (px < 0 || px >= (int)(COLS * TILE_S) || py < 0) return '#';
    if (py >= (int)(ROWS * TILE_S)) return '=';
    return tile_char(read_tile(level_map, px / TILE_S, py / TILE_S));
}

static const char *dir_arrow(float vx, float vy) {
    if (vy > 0.5f)  return "↓";
    if (vy < -0.5f) return "↑";
    if (vx > 0.1f)  return "→";
    if (vx < -0.1f) return "←";
    return "·";
}

// ---------------------------------------------------------------------------
// collision scan – find which tile along an edge is solid
// ---------------------------------------------------------------------------

struct HitInfo {
    bool found;
    int col, row;
    uint8_t type;
};

static HitInfo scan_edge(const uint8_t (*level_map)[COLS],
                         int x0, int y0, int x1, int y1) {
    int dx = (x1 > x0) ? 1 : (x1 < x0) ? -1 : 0;
    int dy = (y1 > y0) ? 1 : (y1 < y0) ? -1 : 0;
    int steps = (dx != 0) ? (x1 - x0) * dx : (y1 - y0) * dy;
    for (int i = 0; i <= steps; i++) {
        int px = x0 + i * dx;
        int py = y0 + i * dy;
        uint8_t t = read_tile(level_map, px / TILE_S, py / TILE_S);
        if (t != 0) {
            return {true, px / TILE_S, py / TILE_S, t};
        }
    }
    // screen edge
    if (x1 < 0 || x1 >= (int)(COLS * TILE_S) || y1 >= (int)(ROWS * TILE_S)) {
        return {true, -1, -1, 0xFF};
    }
    return {false, 0, 0, 0};
}

// ---------------------------------------------------------------------------
// debug_init
// ---------------------------------------------------------------------------

void debug_init() {
    Serial.println(F("\n╔══════════════════════════════╗"));
    Serial.println(F("║     DEBUG MODE ACTIVE       ║"));
    Serial.println(F("╠══════════════════════════════╣"));
    Serial.printf(F("║ Heap free : %6u bytes    ║\n"), ESP.getFreeHeap());
    Serial.printf(F("║ Level     : %u×%u tiles      ║\n"), COLS, ROWS);
    Serial.printf(F("║ Tile size : %u×%u px        ║\n"), TILE_S, TILE_S);
    Serial.println(F("╚══════════════════════════════╝\n"));
}

// ---------------------------------------------------------------------------
// debug_frame
// ---------------------------------------------------------------------------

static unsigned long g_frame = 0;

void debug_frame(const Player &p, const uint8_t (*level_map)[COLS],
                 unsigned long dt_us, float prev_vx, float prev_vy,
                 float prev_x, float prev_y, bool prev_grounded) {

    g_frame++;

    float x  = p.getX();
    float y  = p.getY();
    float vx = p.getVX();
    float vy = p.getVY();
    bool  g  = p.isGrounded();

    static constexpr int W = 10;
    static constexpr int H = 10;

    // ---- tile coordinates (player centre) ----
    int pc = (int)(x + W / 2) / TILE_S;
    int pr = (int)(y + H / 2) / TILE_S;

    // ---- compact line ----
#if DEBUG_COMPACT
    int bl = (int)x;
    int br = (int)(x + W - 1);
    int bt = (int)y;
    int bb = (int)(y + H - 1);

    Serial.printf("[F%5lu] %s (%2d,%2d) P(%3.0f,%3.0f) V(%+5.2f,%+5.2f) %s",
                  g_frame, dir_arrow(vx, vy),
                  pc, pr, x, y, vx, vy,
                  g ? "GND" : "AIR");

    char c_bl = tile_char_at(level_map, bl, bb + 1);
    char c_br = tile_char_at(level_map, br, bb + 1);
    char c_tl = tile_char_at(level_map, bl, bt - 1);
    char c_tr = tile_char_at(level_map, br, bt - 1);

    Serial.printf(" | BL=%c BR=%c TL=%c TR=%c", c_bl, c_br, c_tl, c_tr);

    if (dt_us > 0) {
        float fps = 1000000.0f / (float)dt_us;
        Serial.printf(" %5.1ffps", fps);
    }
    Serial.println();
#endif

    // ---- collision events ----
#if DEBUG_COLLISION
    bool vx_stopped = (prev_vx > 0.1f && vx < 0.1f) || (prev_vx < -0.1f && vx > -0.1f);
    bool vy_landed  = (prev_vy > 0.1f && vy < 0.1f);
    bool vy_ceiled  = (prev_vy < -0.1f && vy > -0.1f);
    bool grnd_changed = (g != prev_grounded);

    if (vx_stopped && prev_vx > 0.1f) {
        int edge_x = (int)(x + W);
        HitInfo hit = scan_edge(level_map, edge_x, (int)y, edge_x, (int)(y + H - 1));
        if (hit.found) {
            Serial.printf("[F%5lu] *** WALL: %+.2f→%+.2f | right(%d) hits tile(%d,%d)=%s\n",
                          g_frame, prev_vx, vx, edge_x, hit.col, hit.row,
                          hit.type == 0xFF ? "SCREEN" : tile_name(hit.type));
        }
    } else if (vx_stopped && prev_vx < -0.1f) {
        int edge_x = (int)x - 1;
        HitInfo hit = scan_edge(level_map, edge_x, (int)y, edge_x, (int)(y + H - 1));
        if (hit.found) {
            Serial.printf("[F%5lu] *** WALL: %+.2f→%+.2f | left(%d) hits tile(%d,%d)=%s\n",
                          g_frame, prev_vx, vx, edge_x, hit.col, hit.row,
                          hit.type == 0xFF ? "SCREEN" : tile_name(hit.type));
        }
    }

    if (vy_landed) {
        int edge_y = (int)(y + H);
        HitInfo hit = scan_edge(level_map, (int)x, edge_y, (int)(x + W - 1), edge_y);
        if (hit.found) {
            Serial.printf("[F%5lu] *** LAND: %+.2f→%+.2f | bot(%d) hits tile(%d,%d)=%s\n",
                          g_frame, prev_vy, vy, edge_y, hit.col, hit.row,
                          hit.type == 0xFF ? "SCREEN" : tile_name(hit.type));
        }
    }

    if (vy_ceiled) {
        int edge_y = (int)y - 1;
        HitInfo hit = scan_edge(level_map, (int)x, edge_y, (int)(x + W - 1), edge_y);
        if (hit.found) {
            Serial.printf("[F%5lu] *** CEIL: %+.2f→%+.2f | top(%d) hits tile(%d,%d)=%s\n",
                          g_frame, prev_vy, vy, edge_y, hit.col, hit.row,
                          hit.type == 0xFF ? "SCREEN" : tile_name(hit.type));
        }
    }

    if (grnd_changed) {
        Serial.printf("[F%5lu] *** GROUND %s (was %s, now %s)\n",
                      g_frame,
                      g ? "GAINED" : "LOST",
                      prev_grounded ? "GND" : "AIR",
                      g ? "GND" : "AIR");
    }
#endif

    // ---- surroundings ----
#if DEBUG_SURROUND
    bool state_changed = vx_stopped || vy_landed || vy_ceiled || grnd_changed;
    if (state_changed || (g_frame % 30 == 0)) {
        int tc = (int)x / TILE_S;
        int tr = (int)y / TILE_S;

        Serial.printf(F("[F%5lu] ═══ surroundings @ tile(%d,%d) ═══\n"),
                      g_frame, tc, tr);

        for (int dr = -1; dr <= 1; dr++) {
            int row = tr + dr;
            Serial.print(F("       "));
            for (int dc = -1; dc <= 1; dc++) {
                int col = tc + dc;
                uint8_t t = read_tile(level_map, col, row);
                bool player_here = (dr >= 0 && dr <= ((y + H - 1) / TILE_S - tr))
                                && (dc >= 0 && dc <= ((x + W - 1) / TILE_S - tc));

                if (t == 0xFF) {
                    Serial.print(F("   "));
                } else if (player_here && dr == 0 && dc == 0) {
                    Serial.printf("[%c]", tile_char(t));
                } else if (t == 0xFE) {
                    Serial.print(F(" = "));
                } else {
                    Serial.printf(" %c ", tile_char(t));
                }
            }
            Serial.println();
        }

        char feet_bl = tile_char_at(level_map, (int)x, (int)(y + H));
        char feet_br = tile_char_at(level_map, (int)(x + W - 1), (int)(y + H));
        Serial.printf("       feet: %c/%c  head: %c/%c  sides: %c/%c\n",
                      feet_bl, feet_br,
                      tile_char_at(level_map, (int)x, (int)y - 1),
                      tile_char_at(level_map, (int)(x + W - 1), (int)y - 1),
                      tile_char_at(level_map, (int)x - 1, (int)(y + H / 2)),
                      tile_char_at(level_map, (int)(x + W), (int)(y + H / 2)));
        Serial.println();
    }
#endif
}

#endif // DEBUG_MODE
