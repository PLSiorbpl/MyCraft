#include "Camera.hpp"

bool camera::operator!=(const camera& other) const {
    return Position != other.Position ||
           Pitch != other.Pitch ||
           Yaw != other.Yaw ||
           Speed != other.Speed ||
           Sensitivity != other.Sensitivity ||
           FirstMouse != other.FirstMouse ||
           LastX != other.LastX ||
           LastY != other.LastY;
}