#include "cutscene.h"

#include <cstddef>
#include <cstring>

static constexpr uint16_t DEFAULT_MIN_DURATION_MS = 2000;
static constexpr uint16_t DEFAULT_MAX_DURATION_MS = 6000;
static constexpr uint16_t DEFAULT_BASE_DURATION_MS = 1500;
static constexpr uint16_t DEFAULT_CHAR_DURATION_MS = 60;

CutscenePlayer::CutscenePlayer(TFT_eSPI &tft)
    : tft(tft),
      scene(nullptr),
      page_index(0),
      page_deadline_ms(0),
      skip_armed(false),
      overlay_dirty(true),
      fired_trigger_mask(0),
      trigger_limit_reported(false) {}

void CutscenePlayer::start(const Cutscene *next_scene) {
  if (isActive()) {
    Serial.println("[CUTSCENE] Ignoring start while active.");
    return;
  }

  if (next_scene == nullptr || next_scene->lines == nullptr ||
      next_scene->line_count == 0) {
    Serial.println("[CUTSCENE] Refusing null or empty scene.");
    return;
  }

  scene = next_scene;
  page_index = 0;
  skip_armed = false;
  beginPage(static_cast<uint32_t>(millis()));

  Serial.printf("[CUTSCENE] Started scene with %u pages.\n",
                static_cast<unsigned>(scene->line_count));
}

bool CutscenePlayer::isActive() const {
  return scene != nullptr;
}

bool CutscenePlayer::update(const InputState &input, uint32_t now_ms) {
  if (!isActive())
    return false;

  if (!input.jump) {
    skip_armed = true;
  } else if (skip_armed) {
    finish(true);
    return false;
  }

  if (static_cast<int32_t>(page_deadline_ms - now_ms) <= 0) {
    ++page_index;
    if (page_index >= scene->line_count) {
      finish(false);
      return false;
    }

    beginPage(now_ms);
  }

  const bool redraw = overlay_dirty;
  overlay_dirty = false;
  return redraw;
}

void CutscenePlayer::maybeTrigger(float player_x) {
  if (isActive())
    return;

  const uint8_t trigger_count =
      CUTSCENE_TRIGGER_COUNT < CutsceneLimits::MAX_TRIGGERS
          ? CUTSCENE_TRIGGER_COUNT
          : CutsceneLimits::MAX_TRIGGERS;

  if (CUTSCENE_TRIGGER_COUNT > CutsceneLimits::MAX_TRIGGERS &&
      !trigger_limit_reported) {
    Serial.printf("[CUTSCENE] Ignoring triggers after limit of %u.\n",
                  static_cast<unsigned>(CutsceneLimits::MAX_TRIGGERS));
    trigger_limit_reported = true;
  }

  for (uint8_t i = 0; i < trigger_count; ++i) {
    const uint8_t bit = static_cast<uint8_t>(1U << i);
    if ((fired_trigger_mask & bit) != 0)
      continue;

    const CutsceneTrigger &trigger = CUTSCENE_TRIGGERS[i];
    if (player_x < trigger.trigger_x)
      continue;

    fired_trigger_mask |= bit;
    start(trigger.scene);
    return;
  }
}

uint8_t CutscenePlayer::stageSprites(RenderSprite *out, uint8_t max) const {
  (void)out;
  (void)max;
  return 0;
}

void CutscenePlayer::renderOverlay() {}

bool CutscenePlayer::isValidActor(ActorId actor) {
  return static_cast<uint8_t>(actor) <
         static_cast<uint8_t>(ActorId::COUNT);
}

uint16_t CutscenePlayer::autoDurationMs(const char *text) {
  const std::size_t length = text == nullptr ? 0 : std::strlen(text);
  const std::size_t max_extra_chars =
      (DEFAULT_MAX_DURATION_MS - DEFAULT_BASE_DURATION_MS) /
      DEFAULT_CHAR_DURATION_MS;

  uint32_t duration = DEFAULT_BASE_DURATION_MS;
  if (length >= max_extra_chars) {
    duration = DEFAULT_MAX_DURATION_MS;
  } else {
    duration += static_cast<uint32_t>(length) * DEFAULT_CHAR_DURATION_MS;
  }

  if (duration < DEFAULT_MIN_DURATION_MS)
    duration = DEFAULT_MIN_DURATION_MS;
  if (duration > DEFAULT_MAX_DURATION_MS)
    duration = DEFAULT_MAX_DURATION_MS;

  return static_cast<uint16_t>(duration);
}

void CutscenePlayer::beginPage(uint32_t now_ms) {
  const DialogueLine &line = scene->lines[page_index];
  const uint16_t duration = line.duration_ms != 0
                                ? line.duration_ms
                                : autoDurationMs(line.text);

  page_deadline_ms = now_ms + static_cast<uint32_t>(duration);
  overlay_dirty = true;
}

void CutscenePlayer::finish(bool skipped) {
  if (scene == nullptr)
    return;

  if (skipped) {
    Serial.printf("[CUTSCENE] Skipped on page %u.\n",
                  static_cast<unsigned>(page_index) + 1U);
  } else {
    Serial.printf("[CUTSCENE] Completed scene after %u pages.\n",
                  static_cast<unsigned>(scene->line_count));
  }

  scene = nullptr;
  page_index = 0;
  page_deadline_ms = 0;
  skip_armed = false;
  overlay_dirty = false;
}
