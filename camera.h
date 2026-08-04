#pragma once
#include <cstdint>

class Camera {
public:
  Camera(float level_w_px);

  void update(float player_x);
  void focusOn(float player_x);
  void reset();
  int offset() const;

private:
  float cam_x;
  float max_cam_x;
  static constexpr float FOLLOW_FRACTION = 0.5f;
};
