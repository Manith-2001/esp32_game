#include "camera.h"
#include "config.h"

Camera::Camera(float level_w_px)
    : cam_x(0), max_cam_x(level_w_px - SCREEN_W) {
  if (max_cam_x < 0)
    max_cam_x = 0;
}

void Camera::update(float player_x) {
  float target = player_x - SCREEN_W * FOLLOW_FRACTION;

  if (target > cam_x)
    cam_x = target;

  if (cam_x < 0)
    cam_x = 0;
  if (cam_x > max_cam_x)
    cam_x = max_cam_x;
}

int Camera::offset() const {
  return (int)cam_x;
}
