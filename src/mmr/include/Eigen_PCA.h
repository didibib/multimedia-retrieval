#include "pmp/visualization/SurfaceMeshGL.h"

#include<Eigen/Dense>
#include<algorithm>

using namespace Eigen;

void PCA_calculate(pmp::SurfaceMeshGL _mesh)
{
    MatrixXf input;
    for (auto v : _mesh.vertices())
        input.col(input.cols() + 1) = VectorXf(_mesh.position(v));
    MatrixXf centered = input.colwise() - input.colwise().mean();
    MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / 2.0f;
    SelfAdjointEigenSolver<MatrixXf> eig(cov);
}