#pragma once

#include "database.h"
#include <pmp/SurfaceMesh.h>
#include <vector>
#include "emd.h"
#include "descriptors.h"

typedef float feature_t;

float e_dist(feature_t *F1, feature_t *F2);
Scalar dist(mmr::Histogram h1, mmr::Histogram h2);
Scalar dist(std::map<std::string, Scalar> data1,
            std::map<std::string, Scalar> data2,
            std::vector<std::string> index);
