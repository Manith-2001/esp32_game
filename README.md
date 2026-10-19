# ESP32 Platformer — Flash Transparent PNG

A Super Mario Bros-style side-scrolling platformer for **ESP32-WROOM-32 (DOIT Devkit V1)** + **ILI9341 320×240 TFT**, written as an Arduino sketch with FreeRTOS, no framebuffer, and no PSRAM.

Features: 60×15 scrolling tile level, player physics, Goomba enemies with stomping, 3 lives + respawn + invulnerability, dialogue cutscenes + victory screen, serial/GPIO input, and serial debug telemetry.

## Hardware

- ESP32-WROOM-32, 4MB flash, ~320KB usable RAM, **no PSRAM**
- ILI9341 320×240 TFT over SPI via `TFT_eSPI` (80 MHz + DMA)
- Physical buttons: GPIO17 (Left), GPIO4 (Right), GPIO18 (Jump), active HIGH
- USB serial @ 115200 baud for debug + keyboard input

## Photos

![Product photo](images/product.jpg)
<!-- Replace with your photo: edit README.md on GitHub, drag-drop the image here -->

![Wiring diagram](images/wiring-diagram.png)
<!-- Replace with your diagram: edit README.md on GitHub, drag-drop the image here -->

## Controls

| Input | Jump | Left | Right |
|---|---|---|---|
| Serial keys | `w` | `a` | `d` |
| Buttons | GPIO18 | GPIO17 | GPIO4 |

Serial keys are single chars, no newline needed, 100 ms hold-decay — resend every ~50-80 ms for continuous movement.

Cutscenes: Right = next page, Left = previous page, `w` = skip. Gameplay is frozen while a scene plays.

Play via terminal:

```bash
screen /dev/ttyUSB0 115200
# type w/a/d, exit with Ctrl-A then K
```

## Build & Flash

No Makefile / PlatformIO — Arduino IDE only:

1. Open `Flash_transparent_PNG.ino` in Arduino IDE
2. Install libraries: `TFT_eSPI`, `PNGdec` (via Library Manager)
3. Configure `TFT_eSPI` for ILI9341 + your wiring
4. Select ESP32 Dev Module, compile, upload

## Architecture

```text
                          BOOT
                           |
                     +-----v------+
                     |  setup()   |
                     | TFT + DMA  |
                     | Mutex+Queue|
                     +-----+------+
                           |
                     +-----v------+
                     | OTP gate   |
                     | once, then |
                     | tasks run  |
                     +-----+------+
                           |
                    +----------------------------------+
                    |           FreeRTOS tasks         |
                    |                                  |
                    |  inputTask (core 0, ~50Hz)       |
                    |   Serial/GPIO -> InputState ─┐   |
                    |                              │   |
                    |  simRenderTask (core 1, 60Hz)│   |
                    |   +----------<--------------┘   |
                    |   |  drain queue (non-blocking)  |
                    |   v                              |
                    |  Player.update ─> Camera ─> Spawn|
                    |    |                |            |
                    |    v                v            |
                    |  Goombas (stomp / collide)       |
                    |    |                            |
                    |    v                            |
                    |  GameState                       |
                    |  (lives / invuln / over/win)     |
                    |    |                            |
                    |    v                            |
                    |  Cutscene? ─yes─> freeze sim,   |
                    |  |             draw pages/actors |
                    |  no                             |
                    |   |                             |
                    |   v                             |
                    |  Renderer: sky + tiles via      |
                    |  pushImage + sprites on top     |
                    |   |                             |
                    +---+-----------------------------+
                        |
                        v
                +-------+--------+
                |  TFT (mutex)   |
                |  Serial debug  |
                +----------------+

  Shared: level_map is read-only (PROGMEM, pgm_read_byte).
          Player is sim-task local. Serial RX (input) / TX (debug)
          share the UART via the driver.
```

- `Flash_transparent_PNG.ino`: thin orchestrator — TFT/DMA init, OTP gate, task startup. `loop()` just sleeps.
- FreeRTOS: `inputTask` (serial poll → queue length-1 always-latest) + `simRenderTask` (fixed tick: input → player → pit/death/lives → camera → viewport render → sprites). All `tft.*` guarded by mutex.
- `config.h`: dimensions — `LEVEL 60×15`, `SCREEN 320×240 / 20×15 cols`, `TILE 16px`.
- Gameplay: `player.{h,cpp}` (gravity/collisions/stomp-bounce), `goomba.{h,cpp}` + `goomba_manager`, `camera` (SMB1 ratchet, right-only scroll), `spawn` (respawn N tiles behind death, search left for ground), `game_state` (3 lives, 5 s post-respawn blink, game-over/victory), `render` (full viewport redraw + transparent `0x0000` → sky), `cutscene` + `cutscenes.cpp` (authored dialogue pages), `victory_screen`, `menu`.
- Sprites: PNGdec decodes `PROGMEM` PNGs (`SpongeBob.h`, `assets/piskel.h`, `player*.h`, `goomba_sprite.h`); tiles are pre-converted RGB565 in `tiles.h` (`BG_SWAP` handles ESP32 SPI byte order).
- Level: `level.h` is `level_map[15][60]` in `PROGMEM`, generated — see below.

## Asset Pipeline

```bash
python3 assets/png_to_rgb565.py img.png [--name var_name] [--bg R,G,B]
python3 assets/convert.py img.png [--name var_name] [-o out.h]
python3 assets/gen_level.py [-s seed] [--c] [--name level_map]
```

`tiles.h` is authoritative for tiles; `assets/*.h` are generated and may be stale.

## Debug Telemetry

In `debug.h`: `DEBUG_MODE 0/1`, sub-flags `DEBUG_COMPACT`, `DEBUG_SURROUND`, `DEBUG_COLLISION`. When off, compiles to nothing.

Per-frame example:

```text
[F   42] · ( 2, 8) P( 34,128) V(+1.00,+0.50) AIR | BL=. BR=B TL=. TR=.  95.2fps
```

Legend: `.`=empty `B`=block `G`=grass `C`=cloud `#`=OOB `=`=pit edge. Includes `*** WALL/LAND/CEIL` and `GROUND LOST/GAINED` events plus 3×3 surroundings dump.

## Project Layout

```text
Flash_transparent_PNG.ino  entrypoint / task startup
config.h                   dimensions
player / physics / camera / spawn / game_state  gameplay
render / tiles.h / level.h  drawing + map
cutscene* / victory_screen / menu  story + UI
serial_input / gpio_input / input*  inputs
rtos_tasks / png_support.ino  tasks + PNG callback
otp_screen  boot gate
assets/  converters + level generator
```
