#pragma once
#include <cmath>

namespace mmr {
namespace param {
static const int TARGET_VALUE = 5000;
static const int BIN_SIZE = 10;
// Max angle
static const float A3_MAX_VALUE = 2 * 3.14;
// Max distance from vertex to centroid
static const float D1_MAX_VALUE = sqrt(2) * .5f;
// Max distance between two vertices
static const float D2_MAX_VALUE = sqrt(2);

} // namespace param
} // namespace mmr