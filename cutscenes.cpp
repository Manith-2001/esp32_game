#include "cutscene.h"

#include "girl_idle.h"
#include "goomba_sprite.h"
#include "manith_sprite.h"

const ActorInfo CUTSCENE_ACTORS[
    static_cast<uint8_t>(ActorId::COUNT)] = {
  {girl_idle, 32, 32, "Mubu"},
  {manith_sprite, 32, 32, "Manith"},
  {goomba_sprite, 16, 16, "Code Bug"},
  {nullptr, 0, 0, ""},
};

static const DialogueLine intro_lines[] = {
  {ActorId::MANITH,
   "What a pleasant day to\ncelebrate your birthday,\ndarling.", 2, {
      {ActorId::GIRL, 112, 128, true},
      {ActorId::MANITH, 176, 128, false},
  }},
  {ActorId::GIRL,
   "I agree, darling. There is no\none to disturb us today.", 2, {
      {ActorId::GIRL, 112, 128, true},
      {ActorId::MANITH, 176, 128, false},
  }},
  {ActorId::GOOMBA,
   "Manith, there are a lot of\nunsolved bugs in your code.\nYou need to fix them right now.", 3, {
      {ActorId::GIRL, 112, 128, true},
      {ActorId::MANITH, 176, 128, false},
      {ActorId::GOOMBA, 240, 144, false},
  }},
  {ActorId::GOOMBA,
   "I am going to kidnap you and\nkeep you hostage until you\nsolve all the bugs.", 3, {
      {ActorId::GIRL, 112, 128, true},
      {ActorId::MANITH, 176, 128, false},
      {ActorId::GOOMBA, 240, 144, false},
  }},
  {ActorId::MANITH, "Oh no, Mubu, save me!", 3, {
      {ActorId::GIRL, 112, 128, true},
      {ActorId::MANITH, 320, 128, false},
      {ActorId::GOOMBA, 336, 144, false},
  }},
  {ActorId::GIRL, "I will have to save my\ndarling Mannu.", 1, {
      {ActorId::GIRL, 112, 128, true},
  }},
};

const Cutscene CUTSCENE_INTRO = {
  intro_lines,
  static_cast<uint8_t>(sizeof(intro_lines) / sizeof(intro_lines[0])),
  CutsceneBackground::FLAT_GROUND
};

// A custom scene requires a DialogueLine array, a Cutscene value, and
// optionally one CutsceneTrigger row. Authors can replace this sample data
// without modifying the playback engine.
static const DialogueLine mid_level_lines[] = {
  {ActorId::NARRATOR, "Something is moving ahead...", 1, {
      {ActorId::GOOMBA, 152, 156, false},
  }},
  {ActorId::MANITH, "There you are!", 2, {
      {ActorId::MANITH, 248, 140, true},
      {ActorId::GOOMBA, 152, 156, false},
  }},
};

static const Cutscene CUTSCENE_MID_LEVEL = {
  mid_level_lines,
  static_cast<uint8_t>(sizeof(mid_level_lines) / sizeof(mid_level_lines[0])),
  CutsceneBackground::REGULAR_LEVEL
};

const CutsceneTrigger CUTSCENE_TRIGGERS[] = {
  {320.0f, &CUTSCENE_MID_LEVEL},
};

const uint8_t CUTSCENE_TRIGGER_COUNT = static_cast<uint8_t>(
    sizeof(CUTSCENE_TRIGGERS) / sizeof(CUTSCENE_TRIGGERS[0]));
