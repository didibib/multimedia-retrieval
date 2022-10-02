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
        auto vp = points[v];
        points[v] += translation;
    }
}

void Norma::pca(SurfaceMesh& mesh) {
    unsigned int n_vertices = mesh.n_vertices();
    MatrixXf input(3, n_vertices);
    auto points = mesh.get_vertex_property<Point>("v:point");
    unsigned int i = 0;
    for (auto v : mesh.vertices())
    {
        input.col(i)[0] = points[v][0];
        input.col(i)[1] = points[v][1];
        input.col(i++)[2] = points[v][2];
    }

    VectorXf mean = input.rowwise().mean();
    MatrixXf centered = input.colwise() - mean;

    MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / (input.rows() - 1);
    SelfAdjointEigenSolver<MatrixXf> eig(cov);

    VectorXf::Index maxv, minv;
    eig.eigenvalues().maxCoeff(&maxv);
    eig.eigenvalues().minCoeff(&minv);

    Matrix3f transfer;
    transfer.col(0) = eig.eigenvectors().col(maxv);
    transfer.col(2) = eig.eigenvectors().col(minv);
    transfer.col(1) = transfer.col(0).cross(transfer.col(2));

    Point pos_temp;
    for (auto v : mesh.vertices())
    {
        Point pos_mesh(points[v]);
        Point pos_temp{0, 0, 0};
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                pos_temp[i] += transfer.col(i)[j] * pos_mesh[j];
        points[v] = pos_temp;
    }
}

void Norma::flip(SurfaceMesh& mesh) {}

void Norma::scale(SurfaceMesh& mesh)
{
    BoundingBox bb = mesh.bounds();
    Point min = bb.min();
    Point max = bb.max();
    Point center = bb.center();
}
} // namespace mmr
