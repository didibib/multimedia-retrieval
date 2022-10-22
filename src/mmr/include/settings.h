#pragma once
#include <cmath>

namespace mmr {
namespace param {
static const int TARGET_VALUE = 5000;
static const int BIN_SIZE = 10;
//
// !! These max values are only valid for normalized meshes !!
//
// Max angle
static const float A3_MAX_VALUE = 7;
// Max distance from vertex to centroid
static const float D1_MAX_VALUE = 1;
// Max distance between two vertices
static const float D2_MAX_VALUE = 2;

static const float D3_MAX_VALUE = sqrt(0.73f);

static const float D4_MAX_VALUE = 1.0f;

} // namespace param
} // namespace mmr