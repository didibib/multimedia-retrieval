#pragma once
#include "pmp/SurfaceMesh.h"

namespace mmr {
class Norma
{
    // Translate barycenter to origin
    static void translate(pmp::SurfaceMesh& mesh);
    // Compute Principal axes and align with coordinate frame
    static void pca(pmp::SurfaceMesh& mesh);
    // Flip based on moment test
    static void flip(pmp::SurfaceMesh& mesh);
    // Scale to unit sized cube
    static void scale(pmp::SurfaceMesh& mesh);

public:
    static void lize(pmp::SurfaceMesh& mesh);
};
} // namespace mmr