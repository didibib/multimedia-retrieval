#include "pmp/visualization/SurfaceMeshGL.h"

#include<Eigen/Dense>
#include<algorithm>

using namespace Eigen;

void PCA_calculate(pmp::SurfaceMeshGL _mesh)
{
    unsigned int n_vertices = _mesh.n_vertices();
    MatrixXf in(n_vertices, 3);
    unsigned int i = 0;
    for (auto v : _mesh.vertices())
        in.row(i++) = VectorXf(_mesh.position(v));
    MatrixXf input = in.transpose();
    RowVectorXf mean = input.colwise().mean();
    MatrixXf centered = input.rowwise() - mean;
    MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / (input.rows() - 1);
    SelfAdjointEigenSolver<MatrixXf> eig(cov);
}