#pragma once

#include "pmp/SurfaceMesh.h"
#include <vector>

namespace mmr {
class Histogram
{
    void create(std::vector<float>& values);
    void normalize();
    int m_minValue = 0;
    int m_maxValue = 0;
    int m_numBins = 0;
    int m_binWidth = 0;

public:
    Histogram(std::vector<float>& values, int num_bins, int min_value, int max_value);
    std::vector<float> histogram;
};

class Descriptor
{
public:
    static pmp::Scalar eccentricity(pmp::SurfaceMesh& mesh);
    static pmp::Scalar compactness(pmp::SurfaceMesh& mesh);
    static Histogram D1(pmp::SurfaceMesh& mesh);
    static Histogram D2(pmp::SurfaceMesh& mesh);
    static Histogram A3(pmp::SurfaceMesh& mesh);
};

} // namespace mmr