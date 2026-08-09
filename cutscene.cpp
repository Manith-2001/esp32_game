#include "cutscene.h"

#include <cstddef>

static constexpr int DIALOGUE_BOX_X = 8;
static constexpr int DIALOGUE_BOX_Y = 172;
static constexpr int DIALOGUE_BOX_W = 304;
static constexpr int DIALOGUE_BOX_H = 60;
static constexpr int DIALOGUE_BOX_RADIUS = 8;
static constexpr uint8_t DIALOGUE_FONT = 2;
static constexpr uint8_t HINT_FONT = 1;
static constexpr int DIALOGUE_LINE_HEIGHT = 18;
static constexpr int DIALOGUE_TEXT_X = 18;
static constexpr int DIALOGUE_TEXT_Y = 184;
static constexpr int DIALOGUE_TEXT_WIDTH = 284;

static constexpr int NAME_TAG_X = 16;
static constexpr int NAME_TAG_Y = 164;
static constexpr int NAME_TAG_H = 16;
static constexpr int NAME_TAG_RADIUS = 4;
static constexpr int NAME_TAG_PADDING_X = 6;
static constexpr uint8_t NAME_TAG_FONT = 1;

static uint8_t wrapText(
    TFT_eSPI &tft,
    const char *text,
    char lines[CutsceneLimits::MAX_TEXT_LINES]
              [CutsceneLimits::TEXT_LINE_CAPACITY],
    bool &truncated) {
  for (uint8_t line = 0; line < CutsceneLimits::MAX_TEXT_LINES; ++line) {
    for (uint8_t character = 0;
         character < CutsceneLimits::TEXT_LINE_CAPACITY;
         ++character) {
      lines[line][character] = '\0';
    }
  }

  truncated = false;
  const char *cursor = text == nullptr ? "" : text;
  uint8_t line_index = 0;
  uint8_t line_length = 0;
  uint8_t line_count = 1;

  while (*cursor != '\0') {
    if (*cursor == '\n') {
      ++cursor;
      if (line_index + 1 >= CutsceneLimits::MAX_TEXT_LINES) {
        truncated = true;
        break;
      }

      ++line_index;
      line_length = 0;
      line_count = static_cast<uint8_t>(line_index + 1);
      continue;
    }

    if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r') {
      ++cursor;
      continue;
    }

    const char *word_start = cursor;
    while (*cursor != '\0' && *cursor != '\n' && *cursor != ' ' &&
           *cursor != '\t' && *cursor != '\r') {
      ++cursor;
    }

    const std::size_t word_length =
        static_cast<std::size_t>(cursor - word_start);
    if (word_length == 0)
      continue;

    bool whole_word_fits = false;
    if (word_length < CutsceneLimits::TEXT_LINE_CAPACITY) {
      const std::size_t separator_length = line_length == 0 ? 0 : 1;
      const std::size_t candidate_length =
          static_cast<std::size_t>(line_length) + separator_length +
          word_length;

      if (candidate_length < CutsceneLimits::TEXT_LINE_CAPACITY) {
        char candidate[CutsceneLimits::TEXT_LINE_CAPACITY];
        std::size_t candidate_index = 0;

        for (uint8_t character = 0; character < line_length; ++character)
          candidate[candidate_index++] = lines[line_index][character];

        if (separator_length != 0)
          candidate[candidate_index++] = ' ';

        for (std::size_t character = 0; character < word_length; ++character)
          candidate[candidate_index++] = word_start[character];

        candidate[candidate_index] = '\0';
        whole_word_fits =
            tft.textWidth(candidate, DIALOGUE_FONT) <= DIALOGUE_TEXT_WIDTH;
      }
    }

    if (whole_word_fits) {
      if (line_length != 0)
        lines[line_index][line_length++] = ' ';

      for (std::size_t character = 0; character < word_length; ++character)
        lines[line_index][line_length++] = word_start[character];

      lines[line_index][line_length] = '\0';
      continue;
    }

    if (line_length != 0) {
      if (line_index + 1 >= CutsceneLimits::MAX_TEXT_LINES) {
        truncated = true;
        break;
      }

      ++line_index;
      line_length = 0;
      line_count = static_cast<uint8_t>(line_index + 1);
    }

    std::size_t word_offset = 0;
    while (word_offset < word_length) {
      while (word_offset < word_length &&
             line_length < CutsceneLimits::TEXT_LINE_CAPACITY - 1) {
        char candidate[CutsceneLimits::TEXT_LINE_CAPACITY];
        for (uint8_t character = 0; character < line_length; ++character)
          candidate[character] = lines[line_index][character];

        candidate[line_length] = word_start[word_offset];
        candidate[line_length + 1] = '\0';

        const bool fits =
            tft.textWidth(candidate, DIALOGUE_FONT) <= DIALOGUE_TEXT_WIDTH;
        if (!fits && line_length != 0)
          break;

        lines[line_index][line_length++] = word_start[word_offset++];
        lines[line_index][line_length] = '\0';
      }

      if (word_offset == word_length)
        break;

      if (line_index + 1 >= CutsceneLimits::MAX_TEXT_LINES) {
        truncated = true;
        break;
      }

      ++line_index;
      line_length = 0;
      line_count = static_cast<uint8_t>(line_index + 1);
    }

    if (truncated)
      break;
  }

  return line_count;
}

static void drawNameTag(TFT_eSPI &tft, const char *name) {
  if (name == nullptr || name[0] == '\0')
    return;

  const int tag_width =
      tft.textWidth(name, NAME_TAG_FONT) + NAME_TAG_PADDING_X * 2;

  tft.fillRoundRect(NAME_TAG_X, NAME_TAG_Y, tag_width, NAME_TAG_H,
                    NAME_TAG_RADIUS, TFT_MAROON);
  tft.drawRoundRect(NAME_TAG_X, NAME_TAG_Y, tag_width, NAME_TAG_H,
                    NAME_TAG_RADIUS, TFT_PINK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_MAROON);
  tft.drawString(name, NAME_TAG_X + tag_width / 2,
                 NAME_TAG_Y + NAME_TAG_H / 2, NAME_TAG_FONT);
}

static void drawNavigationHint(TFT_eSPI &tft) {
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString("LEFT/RIGHT: PREV/NEXT", DIALOGUE_BOX_X + 8,
                 DIALOGUE_BOX_Y + 4, HINT_FONT);
}

static void drawSkipHint(TFT_eSPI &tft) {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString("W: SKIP", DIALOGUE_BOX_X + DIALOGUE_BOX_W - 8,
                 DIALOGUE_BOX_Y + 4, HINT_FONT);
}

CutscenePlayer::CutscenePlayer(TFT_eSPI &tft)
    : tft(tft),
      scene(nullptr),
      page_index(0),
      skip_armed(false),
      left_armed(false),
      right_armed(false),
      overlay_dirty(true),
      truncation_reported(false),
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
  left_armed = false;
  right_armed = false;
  beginPage();

  Serial.printf("[CUTSCENE] Started scene with %u pages.\n",
                static_cast<unsigned>(scene->line_count));
}

bool CutscenePlayer::isActive() const {
  return scene != nullptr;
}

CutsceneBackground CutscenePlayer::background() const {
  return isActive() ? scene->background : CutsceneBackground::REGULAR_LEVEL;
}

bool CutscenePlayer::update(const InputState &input) {
  if (!isActive())
    return false;

  if (!input.jump) {
    skip_armed = true;
  } else if (skip_armed) {
    finish(true);
    return false;
  }

  if (!input.left)
    left_armed = true;
  if (!input.right)
    right_armed = true;

  if (input.left && input.right) {
    // Conflicting direction input must be released before either direction
    // can navigate again.
    left_armed = false;
    right_armed = false;
  } else if (input.right && right_armed) {
    right_armed = false;
    if (page_index + 1U >= scene->line_count) {
      finish(false);
      return false;
    }

    ++page_index;
    beginPage();
  } else if (input.left && left_armed) {
    left_armed = false;
    if (page_index > 0) {
      --page_index;
      beginPage();
    }
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
  if (!isActive() || out == nullptr || max == 0)
    return 0;

  const DialogueLine &line = scene->lines[page_index];
  const uint8_t actor_count =
      line.actor_count < CutsceneLimits::MAX_STAGE_ACTORS
          ? line.actor_count
          : CutsceneLimits::MAX_STAGE_ACTORS;
  uint8_t written = 0;

  for (uint8_t i = 0; i < actor_count && written < max; ++i) {
    const StageActor &stage_actor = line.actors[i];
    if (!isValidActor(stage_actor.actor))
      continue;

    const ActorInfo &actor =
        CUTSCENE_ACTORS[static_cast<uint8_t>(stage_actor.actor)];
    if (actor.pixels == nullptr || actor.width == 0 || actor.height == 0)
      continue;

    RenderSprite &sprite = out[written++];
    sprite.pixels = actor.pixels;
    sprite.x = stage_actor.x;
    sprite.y = stage_actor.y;
    sprite.width = actor.width;
    sprite.height = actor.height;
    sprite.flip_x = stage_actor.flip_x;
  }

  return written;
}

void CutscenePlayer::renderOverlay() {
  if (!isActive())
    return;

  const DialogueLine &line = scene->lines[page_index];
  char lines[CutsceneLimits::MAX_TEXT_LINES]
             [CutsceneLimits::TEXT_LINE_CAPACITY];
  bool truncated = false;
  const uint8_t line_count = wrapText(tft, line.text, lines, truncated);

  tft.fillRoundRect(DIALOGUE_BOX_X, DIALOGUE_BOX_Y, DIALOGUE_BOX_W,
                    DIALOGUE_BOX_H, DIALOGUE_BOX_RADIUS, TFT_NAVY);
  tft.drawRoundRect(DIALOGUE_BOX_X, DIALOGUE_BOX_Y, DIALOGUE_BOX_W,
                    DIALOGUE_BOX_H, DIALOGUE_BOX_RADIUS, TFT_WHITE);

  if (isValidActor(line.speaker) && line.speaker != ActorId::NARRATOR) {
    const ActorInfo &speaker =
        CUTSCENE_ACTORS[static_cast<uint8_t>(line.speaker)];
    drawNameTag(tft, speaker.name);
  }

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  for (uint8_t i = 0; i < line_count; ++i) {
    tft.drawString(lines[i], DIALOGUE_TEXT_X,
                   DIALOGUE_TEXT_Y + static_cast<int>(i) *
                                        DIALOGUE_LINE_HEIGHT,
                   DIALOGUE_FONT);
  }

  drawNavigationHint(tft);
  drawSkipHint(tft);

  if (truncated && !truncation_reported) {
    Serial.printf("[CUTSCENE] Text truncated on page %u.\n",
                  static_cast<unsigned>(page_index) + 1U);
    truncation_reported = true;
  }
}

bool CutscenePlayer::isValidActor(ActorId actor) {
  return static_cast<uint8_t>(actor) <
         static_cast<uint8_t>(ActorId::COUNT);
}

void CutscenePlayer::beginPage() {
  overlay_dirty = true;
  truncation_reported = false;
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
  skip_armed = false;
  left_armed = false;
  right_armed = false;
  overlay_dirty = false;
  truncation_reported = false;
}
