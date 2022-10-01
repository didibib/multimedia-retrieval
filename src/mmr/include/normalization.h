#pragma once
#include "pmp/SurfaceMesh.h"

namespace mmr {
namespace normalization {
// Translate barycenter to origin
void translate(pmp::SurfaceMesh& mesh);
// Compute Principal axes and align with coordinate frame
void pca(pmp::SurfaceMesh& mesh);
// Flip based on moment test
void flip(pmp::SurfaceMesh& mesh);
// Scale to unit sized cube
void scale(pmp::SurfaceMesh& mesh);
} // namespace normalization
} // namespace mmr