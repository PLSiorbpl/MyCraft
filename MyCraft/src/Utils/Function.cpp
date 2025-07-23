#include "Function.hpp"

float Fun::ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) {
    float fovX_rad = glm::radians(fovX_deg);
    float fovY_rad = 2.0f * atan(tan(fovX_rad / 2.0f) / aspectRatio);
    return glm::degrees(fovY_rad);
}
