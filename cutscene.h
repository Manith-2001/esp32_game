#pragma once

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "input.h"
#include "render.h"

#include <cstdint>

namespace CutsceneLimits {
static constexpr uint8_t MAX_STAGE_ACTORS = 4;
static constexpr uint8_t MAX_TRIGGERS = 8;
static constexpr uint8_t MAX_TEXT_LINES = 3;
static constexpr uint8_t TEXT_LINE_CAPACITY = 48;
}

enum class ActorId : uint8_t {
  GIRL = 0,
  MANITH,
  GOOMBA,
  NARRATOR,
  COUNT
};

struct ActorInfo {
  const uint16_t *pixels;
  uint8_t width;
  uint8_t height;
  const char *name;
};

struct StageActor {
  ActorId actor;
  int16_t x;
  int16_t y;
  bool flip_x;
};

struct DialogueLine {
  ActorId speaker;
  const char *text;
  uint16_t duration_ms;
  uint8_t actor_count;
  StageActor actors[CutsceneLimits::MAX_STAGE_ACTORS];
};

struct Cutscene {
  const DialogueLine *lines;
  uint8_t line_count;
};

struct CutsceneTrigger {
  float trigger_x;
  const Cutscene *scene;
};

extern const ActorInfo CUTSCENE_ACTORS[
    static_cast<uint8_t>(ActorId::COUNT)];
extern const Cutscene CUTSCENE_INTRO;
extern const CutsceneTrigger CUTSCENE_TRIGGERS[];
extern const uint8_t CUTSCENE_TRIGGER_COUNT;

class CutscenePlayer {
public:
  explicit CutscenePlayer(TFT_eSPI &tft);

  void start(const Cutscene *scene);
  bool isActive() const;
  bool update(const InputState &input, uint32_t now_ms);
  void maybeTrigger(float player_x);
  uint8_t stageSprites(RenderSprite *out, uint8_t max) const;
  void renderOverlay();

private:
  static bool isValidActor(ActorId actor);
  static uint16_t autoDurationMs(const char *text);
  void beginPage(uint32_t now_ms);
  void finish(bool skipped);

  TFT_eSPI &tft;
  const Cutscene *scene;
  uint8_t page_index;
  uint32_t page_deadline_ms;
  bool skip_armed;
  bool overlay_dirty;
  uint8_t fired_trigger_mask;
  bool trigger_limit_reported;
};
