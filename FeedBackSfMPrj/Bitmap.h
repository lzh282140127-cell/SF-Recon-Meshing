#pragma once
#include <algorithm>
#include <cmath>
#include <ios>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
#include "basestring.h"


// Jet colormap inspired by Matlab. Grayvalues are expected in the range [0, 1]
// and are converted to RGB values in the same range.
class JetColormap {
public:
    static float Red(float gray);
    static float Green(float gray);
    static float Blue(float gray);

private:
    static float Interpolate(float val, float y0, float x0, float y1, float x1);
    static float Base(float val);
};








