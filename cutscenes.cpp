#include "cutscene.h"

#include "girl_idle.h"
#include "goomba_sprite.h"
#include "manith_sprite.h"

const ActorInfo CUTSCENE_ACTORS[
    static_cast<uint8_t>(ActorId::COUNT)] = {
  {girl_idle, 32, 32, "Mubu"},
  {manith_sprite, 32, 32, "Manith"},
  {goomba_sprite, 16, 16, "Goomba"},
  {nullptr, 0, 0, ""},
};

static const DialogueLine intro_lines[] = {
  {ActorId::GIRL, "Manith! The goombas escaped the pen!", 0, 2, {
      {ActorId::GIRL, 40, 140, false},
      {ActorId::MANITH, 248, 140, true},
  }},
  {ActorId::MANITH, "Then we round them up. Together.", 0, 2, {
      {ActorId::GIRL, 40, 140, false},
      {ActorId::MANITH, 248, 140, true},
  }},
  {ActorId::GOOMBA, "...", 0, 2, {
      {ActorId::MANITH, 248, 140, true},
      {ActorId::GOOMBA, 120, 156, false},
  }},
};

const Cutscene CUTSCENE_INTRO = {
  intro_lines,
  static_cast<uint8_t>(sizeof(intro_lines) / sizeof(intro_lines[0]))
};

// A custom scene requires a DialogueLine array, a Cutscene value, and
// optionally one CutsceneTrigger row. Authors can replace this sample data
// without modifying the playback engine.
static const DialogueLine mid_level_lines[] = {
  {ActorId::NARRATOR, "Something is moving ahead...", 0, 1, {
      {ActorId::GOOMBA, 152, 156, false},
  }},
  {ActorId::MANITH, "There you are!", 0, 2, {
      {ActorId::MANITH, 248, 140, true},
      {ActorId::GOOMBA, 152, 156, false},
  }},
};

static const Cutscene CUTSCENE_MID_LEVEL = {
  mid_level_lines,
  static_cast<uint8_t>(sizeof(mid_level_lines) / sizeof(mid_level_lines[0]))
};

const CutsceneTrigger CUTSCENE_TRIGGERS[] = {
  {320.0f, &CUTSCENE_MID_LEVEL},
};

const uint8_t CUTSCENE_TRIGGER_COUNT = static_cast<uint8_t>(
    sizeof(CUTSCENE_TRIGGERS) / sizeof(CUTSCENE_TRIGGERS[0]));
