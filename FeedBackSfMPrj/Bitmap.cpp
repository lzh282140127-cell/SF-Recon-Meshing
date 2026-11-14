#include "Bitmap.h"

#include <regex>
#include <unordered_map>



float JetColormap::Red(const float gray) { return Base(gray - 0.25f); }

float JetColormap::Green(const float gray) { return Base(gray); }

float JetColormap::Blue(const float gray) { return Base(gray + 0.25f); }

float JetColormap::Base(const float val) {
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (val <= 0.125f) {
        return 0.0f;
    }
    else if (val <= 0.375f) {
        return Interpolate(2.0f * val - 1.0f, 0.0f, -0.75f, 1.0f, -0.25f);
    }
    else if (val <= 0.625f) {
        return 1.0f;
    }
    else if (val <= 0.87f) {
        return Interpolate(2.0f * val - 1.0f, 1.0f, 0.25f, 0.0f, 0.75f);
    }
    else {
        return 0.0f;
    }
}

float JetColormap::Interpolate(const float val,
    const float y0,
    const float x0,
    const float y1,
    const float x1) {
    return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
}