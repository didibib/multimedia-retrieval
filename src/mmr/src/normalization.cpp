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

    unsigned int i = 0;
    for (auto v : mesh.vertices())
    {
        input.row(i)[0] = mesh.position(v)[0];
        input.row(i)[1] = mesh.position(v)[1];
        input.row(i++)[2] = mesh.position(v)[2];
    }

    VectorXf mean = input.rowwise().mean();
    MatrixXf centered = input.colwise() - mean;

    MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / (input.rows() - 1);
    SelfAdjointEigenSolver<MatrixXf> eig(cov);

    VectorXf::Index maxv, minv;
    float val = eig.eigenvalues().maxCoeff(&maxv);
    eig.eigenvalues().minCoeff(&minv);

    Matrix3f transfer;
    transfer.col(0) = eig.eigenvectors().col(maxv);
    transfer.col(2) = eig.eigenvectors().col(minv);
    transfer.col(1) =
        eig.eigenvectors().col(minv).cross(eig.eigenvectors().col(maxv));

    Vector3f pos_temp;
    for (auto v : mesh.vertices())
    {
        pos_temp[0] = mesh.position(v)[0];
        pos_temp[1] = mesh.position(v)[1];
        pos_temp[2] = mesh.position(v)[2];
        pos_temp *= transfer;
        pos_temp /= val;
        mesh.position(v)[0] = pos_temp[0];
        mesh.position(v)[1] = pos_temp[1];
        mesh.position(v)[2] = pos_temp[2];
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
