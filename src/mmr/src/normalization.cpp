#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/MatVec.h>
#include "normalization.h"

using namespace pmp;
using namespace Eigen;

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
        Point vp = points[v];
        points[v] += translation;
    }
}

void Norma::pca(SurfaceMesh& mesh) {}

void Norma::flip(SurfaceMesh& mesh) {}

void Norma::scale(SurfaceMesh& mesh)
{
    BoundingBox bb = mesh.bounds();
    Point center = bb.center();

    Point scale = (bb.max() - bb.min());
    scale[0] = scale[0] > 0.0f ? 1.f / scale[0] : 1.f;
    scale[1] = scale[0] > 0.0f ? 1.f / scale[1] : 1.f;
    scale[2] = scale[0] > 0.0f ? 1.f / scale[2] : 1.f;

    Transform<float, 3, Affine> T = Transform<float, 3, Affine>::Identity();
    T.scale(Vector3f(scale[0], scale[1], scale[2]));
    T.translate(Vector3f(center[0], center[1], center[2]));

    auto points = mesh.get_vertex_property<Point>("v:point");
    for (auto v : mesh.vertices())
    {
        Vector3f p(points[v][0], points[v][1], points[v][2]);

        auto result = T * p;

        points[v][0] = result[0];
        points[v][1] = result[1];
        points[v][2] = result[2];
    }
}
} // namespace mmr
