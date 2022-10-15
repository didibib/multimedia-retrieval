#include "descriptors.h"
#include "util.h"
#include <pmp/algorithms/DifferentialGeometry.h>
#include <random>
#include <chrono>

namespace mmr {
Histogram::Histogram(std::vector<float>& values, int bins, int max_value) {

}

void Histogram::create() {}

void Histogram::normalize() {}

pmp::Scalar Descriptor::eccentricity(pmp::SurfaceMesh& mesh)
{
    unsigned int n_vertices = mesh.n_vertices();
    pmp::Point center = pmp::centroid(mesh);
    Eigen::MatrixXf input(3, n_vertices);
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    unsigned int i = 0;
    for (auto v : mesh.vertices())
    {
        input.col(i)[0] = points[v][0] - center[0];
        input.col(i)[1] = points[v][1] - center[1];
        input.col(i++)[2] = points[v][2] - center[2];
    }

    Eigen::VectorXf mean = input.rowwise().mean();
    Eigen::MatrixXf centered = input.colwise() - mean;
    Eigen::MatrixXf cov = centered * centered.adjoint();
    cov = cov.array() / (input.rows() - 1);
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
    Eigen::VectorXf::Index maxv, minv;
    eig.eigenvalues().maxCoeff(&maxv);
    eig.eigenvalues().minCoeff(&minv);
    pmp::Scalar ecc = eig.eigenvalues()[minv] / eig.eigenvalues()[maxv];
    ecc *= ecc > 0 ? 1.f : -1.f;
    return ecc;
}

pmp::Scalar Descriptor::compactness(pmp::SurfaceMesh& mesh)
{
    auto S = surface_area(mesh);
    auto V = volume(mesh);
    return (S * S * S) / (V * V * 36 * M_PI);
}

Histogram Descriptor::A3(pmp::SurfaceMesh& mesh)
{
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random = std::bind(std::uniform_int_distribution<int>(0, TARGET_VALUE - 1),
                            std::mt19937(seed));

    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    int k = pow(TARGET_VALUE, 1.0 / 3.0);

    // Use pointers to create create data on heap
    std::vector<float>* angles = new std::vector<float>();
    angles->reserve(TARGET_VALUE);
    int max_value = 0;

    for (unsigned int i = 0; i < k; i++)
    {
        int v1 = random();

        for (unsigned int j = 0; j < k; j++)
        {
            int v2 = random();
            while (v2 == v1)
                v2 = random();

            for (unsigned int l = 0; l < k; l++)
            {
                int v3 = random();
                while (v3 == v2 || v3 == v1)
                    v3 = random();

                pmp::Point p1 = points[Vertex(v1)];
                pmp::Point p2 = points[Vertex(v2)];
                pmp::Point p3 = points[Vertex(v3)];

                auto u = p2 - p1;
                auto v = p3 - p1;

                float dot = pmp::dot(u, v);
                float u_mag = pmp::distance(p2, p1);
                float v_mag = pmp::distance(p3, p1);

                float angle = acos(dot / (u_mag * v_mag));

                if (angle > max_value)
                    max_value = angle;

                angles->push_back(angle);
            }
        }
    }

    return Histogram(*angles, 10, max_value);
}
} // namespace mmr