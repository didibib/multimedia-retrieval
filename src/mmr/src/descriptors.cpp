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


pmp::Scalar Descriptor::diameter(pmp::SurfaceMesh& mesh)
{
    unsigned int n_vertices = mesh.n_vertices();
    pmp::Point center = pmp::centroid(mesh);
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    float maxDiameter = 0.0f;
    float maxDistCentroid = 0.0f;
    for (auto v : mesh.vertices())
    {
        float distCentroid = pmp::distance(points[v], center);

        /*if (distCentroid > maxDistCentroid)
            maxDistCentroid = distCentroid;
        else
            continue;*/

        for (auto u : mesh.vertices())
        {
            if (v == u)
                continue;
            float dist = pmp::distance(points[v], points[u]);
            if (dist > maxDiameter)
                maxDiameter = dist;
        }
    }
    return maxDiameter;
}

Histogram Descriptor::D1(pmp::SurfaceMesh& mesh)
{
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    auto center = centroid(mesh);
    std::vector<Scalar>* D1 = new std::vector<float>();
    D1->reserve(TARGET_VALUE);
    for (size_t i = 0; i < TARGET_VALUE; i++)
    {
        size_t v = random();
        D1->push_back(distance(center, points[Vertex(v)]));
    }
    return Histogram(*D1, 10, sqrt(1/2));
}

Histogram Descriptor::D2(pmp::SurfaceMesh& mesh)
{
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    auto center = centroid(mesh);
    std::vector<Scalar>* D2 = new std::vector<float>();
    D2->reserve(TARGET_VALUE);

    for (size_t i = 0; i < TARGET_VALUE; i++)
    {
        size_t v1 = random();
        for (size_t j = 0; j < TARGET_VALUE; j++)
        {
            size_t v2 = random();
            D2->push_back(distance(points[Vertex(v1)], points[Vertex(v2)]));
        }
    }
    return Histogram(*D2, 10, sqrt(2));
}


Histogram Descriptor::D3(pmp::SurfaceMesh& mesh)
{
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    
    std::vector<Scalar>* D3 = new std::vector<float>();
    D3->reserve(TARGET_VALUE);

    for (size_t i = 0; i < TARGET_VALUE; i++)
    {
        size_t r1 = random();
        for (size_t j = 0; j < TARGET_VALUE; j++)
        {
            if (i == j)
                continue;
            size_t r2 = random();
            for (size_t k = 0; k < TARGET_VALUE; k++)
            {
                if (i == k || j == k)
                    continue;
                size_t r3 = random();
                Point v1 = points[Vertex(r1)];
                Point v2 = points[Vertex(r2)];
                Point v3 = points[Vertex(r3)];
                Point v1v2 = v2 - v1;
                Point v1v3 = v3 - v1;
                float dist12 = norm(v1v2);
                float dist13 = norm(v1v3);
                float theta = acosf(dot(v1v2, v1v3) / (dist12 * dist13));
                D3->push_back(sqrt(0.5f * dist12 * dist13 * sinf(theta)));
            }


        }
    }
    return Histogram(*D3, 10, sqrt(2.83f));
}


Histogram Descriptor::D4(pmp::SurfaceMesh& mesh)
{
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    std::vector<Scalar>* D4 = new std::vector<float>();
    D4->reserve(TARGET_VALUE);

    for (size_t i = 0; i < TARGET_VALUE; i++)
    {
        size_t r1 = random();
        for (size_t j = 0; j < TARGET_VALUE; j++)
        {
            if (i == j)
                continue;
            size_t r2 = random();
            for (size_t k = 0; k < TARGET_VALUE; k++)
            {
                if (i == k || j == k)
                    continue;
                size_t r3 = random();
                for (size_t u = 0; u < TARGET_VALUE; u++)
                {
                    if (i == u || j == u || k == u)
                        continue;
                    size_t r4 = random();
                    Point v1 = points[Vertex(r1)];
                    Point v2 = points[Vertex(r2)];
                    Point v3 = points[Vertex(r3)];
                    Point v4 = points[Vertex(r4)];
                    Eigen::Matrix4f verts;
                    verts.row(0) << v1[0], v1[1], v1[2], 1;
                    verts.row(1) << v2[0], v2[1], v2[2], 1;
                    verts.row(2) << v3[0], v3[1], v3[2], 1;
                    verts.row(3) << v4[0], v4[1], v4[2], 1;
                    D4->push_back(abs(verts.determinant()) / 6);
                }
                
            }
        }
    }
    return Histogram(*D4, 10, cbrt(8));
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