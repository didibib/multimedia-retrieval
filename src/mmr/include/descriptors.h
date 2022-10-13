#pragma once

#include "pmp/SurfaceMesh.h"

namespace mmr {
class Histogram
{
public:
    Histogram(int bins);


};

class Descriptor
{
public:
    static pmp::Scalar eccentricity(pmp::SurfaceMesh& mesh);
    static pmp::Scalar compactness(pmp::SurfaceMesh& mesh);
    static Histogram A3(pmp::SurfaceMesh& mesh);
};

} // namespace mmr