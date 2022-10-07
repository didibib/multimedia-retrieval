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

void Norma::pca(SurfaceMesh& mesh)
{
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
    transfer.col(2) *= transfer.determinant() > 0 ? 1 : -1;

    for (auto v : mesh.vertices())
    {
        Point pos_mesh(points[v]);
        Point pos_temp{0, 0, 0};
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                pos_temp[i] += transfer(j, i) * pos_mesh[j];
        points[v] = pos_temp;
    }
}

void Norma::flip(SurfaceMesh& mesh)
{
    auto points = mesh.get_vertex_property<Point>("v:point");

    // Sum f_i
    float flip_x = 0;
    float flip_y = 0;
    float flip_z = 0;
    for (auto f : mesh.faces())
    {
        Point center(0);
        int vertices = 0;
        for (auto v : mesh.vertices(f))
        {
            center += points[v];
            vertices++;
        }
        center /= vertices;

        int sign_x = center[0] >= 0 ? 1 : -1;
        int sign_y = center[1] >= 0 ? 1 : -1;
        int sign_z = center[2] >= 0 ? 1 : -1;
        flip_x += sign_x * center[0] * center[0];
        flip_y += sign_y * center[1] * center[1];
        flip_z += sign_z * center[2] * center[2];
    }

    // Flip
    for (auto v : mesh.vertices())
    {
        points[v][0] *= flip_x >= 0 ? 1 : -1;
        points[v][1] *= flip_y >= 0 ? 1 : -1;
        points[v][2] *= flip_z >= 0 ? 1 : -1;
    }
}

void Norma::scale(SurfaceMesh& mesh)
{
    BoundingBox bb = mesh.bounds();
    Point center = bb.center();

    Point scale = (bb.max() - bb.min());
    scale[0] = std::max(scale[0], std::max(scale[1], scale[2]));
    scale[2] = scale[1] = scale[0] = scale[0] > 0.0f ? 1.f / scale[0] : 1.0f;

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
