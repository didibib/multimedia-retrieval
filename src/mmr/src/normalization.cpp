#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/MatVec.h>
#include "normalization.h"

using namespace pmp;

namespace mmr {

void Norma::lize(SurfaceMesh& mesh)
{
    translate(mesh);
    pca(mesh);
    flip(mesh);
    scale(mesh);
}

void Norma::translate(SurfaceMesh& mesh)
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

void Norma::pca(SurfaceMesh& mesh) {}

void Norma::flip(SurfaceMesh& mesh) {}

void Norma::scale(SurfaceMesh& mesh)
{
    BoundingBox bb = mesh.bounds();
    Point min = bb.min();
    Point max = bb.max();
    Point center = bb.center();
}
} // namespace mmr
