#include "pmp/visualization/SurfaceMeshGL.h"

#include<Eigen/Dense>
#include<algorithm>

using namespace Eigen;

void pca_calculate(pmp::SurfaceMeshGL _mesh)
{
    unsigned int n_vertices = _mesh.n_vertices();
    MatrixXf input(3, n_vertices);
    unsigned int i = 0;
    for (auto v : _mesh.vertices())
    {
        input.row(i)[0] = _mesh.position(v)[0];
        input.row(i)[1] = _mesh.position(v)[1];
        input.row(i++)[2] = _mesh.position(v)[2];
    }
    VectorXf mean = input.rowwise().mean();
    MatrixXf centered = input.colwise() - mean;
    MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / (input.rows() - 1);
    SelfAdjointEigenSolver<MatrixXf> eig(cov);
}
