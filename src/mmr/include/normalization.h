#pragma once
#include "pmp/SurfaceMesh.h"

namespace mmr {
class Normalize
{
public:
    // Subdivide low vertices meshes (<10000)
    static void remesh(pmp::SurfaceMesh& mesh);
    // Translate barycenter to origin
    static void translate(pmp::SurfaceMesh& mesh);
    // Compute Principal axes and align with coordinate frame
    static void pca_pose(pmp::SurfaceMesh& mesh);
    // Flip based on moment test
    static void flip(pmp::SurfaceMesh& mesh);
    // Scale to unit sized cube
    static void scale(pmp::SurfaceMesh& mesh);

    static void all_steps(pmp::SurfaceMesh& mesh);
};
} // namespace mmr