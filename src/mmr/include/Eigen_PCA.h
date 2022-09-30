#include "pmp/visualization/SurfaceMeshGL.h"

#include<iostream>
#include<Eigen/Dense>
#include<algorithm>

using namespace std;
using namespace Eigen;
using namespace pmp;

void PCA_calculate(SurfaceMeshGL _mesh) {
    Matrix3Xf input;
    Matrix3f cov;
    MatrixXf temp;
    
    for (auto v : _mesh.vertices())
    {
        temp(1) = _mesh.position(v)[1];
        temp(2) = _mesh.position(v)[2];
        temp(3) = _mesh.position(v)[3];
        input.col(input.cols() + 1) = temp;
    }
        
    Matrix3Xf centered = input.colwise() - input.colwise().mean();
    cov = centered * centered.adjoint();
    cov = cov.array() / 2.0f;
    SelfAdjointEigenSolver<Matrix3f> eig(cov);
}