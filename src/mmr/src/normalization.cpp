#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/MatVec.h>
#include "normalization.h"

using namespace pmp;

namespace mmr {
namespace normalization {
void translate(SurfaceMesh& mesh)
{
    Point origin(0, 0, 0);
    Point center = centroid(mesh);
    Point translation = origin - center;

    auto points = mesh.get_vertex_property<Point>("v:point");

    for (auto v : mesh.vertices())
    {
        auto vp = points[v];
        points[v] += translation;
    }
}

void pca(SurfaceMesh& mesh) {}

void flip(SurfaceMesh& mesh) {}

void scale(SurfaceMesh& mesh) {
    BoundingBox bounds = mesh.bounds();

}
} // namespace normalization
} // namespace mmr
