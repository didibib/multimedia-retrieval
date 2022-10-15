#pragma once

#include "pmp/SurfaceMesh.h"
#include <vector>

namespace mmr {
class Histogram
{
    struct Bin
    {
        pmp::vec2 range;
        float value;    
    };

    void create();
    void normalize();
    int m_maxValue = 0;
    int m_binSize = 0;

public:
    Histogram(std::vector<float>& values, int bins, int max_value);

    std::vector<Bin> histogram;
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