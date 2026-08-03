#pragma once
#include <cinttypes>
#include <string>

class Fun {
public:
    static float ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio);

    static std::string FormatNumber(uint64_t n);

    static std::string FormatSize(uint64_t n);
};