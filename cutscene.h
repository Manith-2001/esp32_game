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

enum class CutsceneBackground : uint8_t {
  REGULAR_LEVEL,
  FLAT_GROUND
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
  uint8_t actor_count;
  StageActor actors[CutsceneLimits::MAX_STAGE_ACTORS];
};

struct Cutscene {
  const DialogueLine *lines;
  uint8_t line_count;
  CutsceneBackground background;
  bool ends_game;
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
  bool completesGame() const;
  CutsceneBackground background() const;
  bool update(const InputState &input);
  void maybeTrigger(float player_x);
  void resetTriggers();
  uint8_t stageSprites(RenderSprite *out, uint8_t max) const;
  void renderOverlay();

private:
  static bool isValidActor(ActorId actor);
  void beginPage();
  void finish(bool skipped);

  TFT_eSPI &tft;
  const Cutscene *scene;
  uint8_t page_index;
  bool skip_armed;
  bool left_armed;
  bool right_armed;
  bool overlay_dirty;
  bool truncation_reported;
  uint8_t fired_trigger_mask;
  bool trigger_limit_reported;
};
