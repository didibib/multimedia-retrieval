#pragma once

#include "pmp/SurfaceMesh.h"
#include <vector>

namespace mmr {
class Histogram
{
    void create(std::vector<float>& values);
    void normalize();
    float m_minValue = 0;
    float m_maxValue = 0;
    int m_numBins = 0;
    float m_binWidth = 0;

public:
    Histogram(std::vector<float>& values, float min_value, float max_value,
              int num_bins);
    std::vector<float> histogram;
};

class Descriptor
{
public:
    static pmp::Scalar eccentricity(pmp::SurfaceMesh& mesh);
    static pmp::Scalar compactness(pmp::SurfaceMesh& mesh);
    static pmp::Scalar diameter(pmp::SurfaceMesh& mesh);
    static Histogram D1(pmp::SurfaceMesh& mesh);
    static Histogram D2(pmp::SurfaceMesh& mesh);
    static Histogram A3(pmp::SurfaceMesh& mesh);
};

} // namespace mmr