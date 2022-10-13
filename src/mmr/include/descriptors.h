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

    static Histogram A3(pmp::SurfaceMesh& mesh);
};

} // namespace mmr