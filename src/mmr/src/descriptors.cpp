#include "descriptors.h"
#include "settings.h"
#include "util.h"
#include <pmp/algorithms/DifferentialGeometry.h>
#include <random>
#include <chrono>
#include <fstream>

namespace mmr {

Histogram::Histogram(std::string name_, std::vector<float>& values,
                     float min_value, float max_value, int num_bins)
{
    name = name_;
    m_minValue = min_value;
    m_maxValue = max_value;
    m_numBins = num_bins;
    m_binWidth = static_cast<float>(m_maxValue - m_minValue) / (m_numBins - 1);

    histogram.resize(m_numBins);
    m_bins.resize(m_numBins);

    for (size_t i = 0; i < m_numBins; i++)
        m_bins[i] = i * m_binWidth;

    create(values);
    normalize();
}

void Histogram::save(std::string filename)
{
    std::ofstream fout;
    fout.open(util::getExportDir("histogram/data/") + filename);
    // 
    // !! If you add data or change the order of lines, you also need update the python file !!
    //  
    /*[0]*/ fout << name << "\n";
    /*[1]*/ fout << m_minValue << "\n";
    /*[2]*/ fout << m_maxValue << "\n";
    /*[3]*/ fout << m_binWidth << "\n";

    for (size_t i = 0; i < m_bins.size(); i++)
        /*[4]*/ fout << m_bins[i] << " ";

    fout << "\n";

    for (size_t i = 0; i < histogram.size(); i++)
        /*[5]*/ fout << histogram[i] << " ";

    fout.close();
}

void Histogram::create(std::vector<float>& values)
{
    for (unsigned int i = 0; i < values.size(); i++)
    {
        auto idx = std::floorf((values[i] - m_minValue) / m_binWidth);
        int index = static_cast<int>(idx);
        histogram[index]++;
    }
}

void Histogram::normalize()
{
    float maxValue = 0;
    for (unsigned int i = 0; i < histogram.size(); i++)
        if (histogram[i] > maxValue)
            maxValue = histogram[i];

    for (unsigned int i = 0; i < histogram.size(); i++)
        histogram[i] /= maxValue;
}

pmp::Scalar Descriptor::eccentricity(pmp::SurfaceMesh& mesh)
{
    size_t n_vertices = mesh.n_vertices();
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
    size_t n_vertices = mesh.n_vertices();
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

Histogram Descriptor::A3(pmp::SurfaceMesh& mesh)
{
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh.n_vertices() - 1),
                  std::mt19937(seed));

    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    // Use pointers to create create data on heap
    std::vector<float>* angles = new std::vector<float>();
    angles->reserve(param::TARGET_VALUE);
    int max_value = 0;

    for (unsigned int i = 0; i < param::TARGET_VALUE; i++)
    {
        int v1 = random();

        int v2 = random();
        while (v2 == v1)
            v2 = random();

        int v3 = random();
        while (v3 == v2 || v3 == v1)
            v3 = random();

        pmp::Point p1 = points[pmp::Vertex(v1)];
        pmp::Point p2 = points[pmp::Vertex(v2)];
        pmp::Point p3 = points[pmp::Vertex(v3)];

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

    return Histogram("A3", *angles, 0, max_value, param::BIN_SIZE);
}

Histogram Descriptor::D1(pmp::SurfaceMesh& mesh)
{
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh.n_vertices() - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    auto center = centroid(mesh);
    std::vector<float>* D1 = new std::vector<float>();
    D1->reserve(param::TARGET_VALUE);
    for (size_t i = 0; i < param::TARGET_VALUE; i++)
    {
        size_t v = random();
        D1->push_back(distance(center, points[pmp::Vertex(v)]));
    }
    return Histogram("D1", *D1, 0, sqrt(1 / 2), param::BIN_SIZE);
}

Histogram Descriptor::D2(pmp::SurfaceMesh& mesh)
{
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh.n_vertices() - 1),
                  std::mt19937(seed));

    auto points = mesh.get_vertex_property<pmp::Point>("v:point");
    auto center = centroid(mesh);
    std::vector<float>* D2 = new std::vector<float>();
    D2->reserve(param::TARGET_VALUE);

    for (size_t i = 0; i < param::TARGET_VALUE; i++)
    {
        size_t v1 = random();
        size_t v2 = random();
        while (v2 == v1)
            v2 = random();
        Point p1 = points[pmp::Vertex(v1)];
        Point p2 = points[pmp::Vertex(v2)];
        D2->push_back(distance(p1, p2));
    }
    return Histogram("D2", *D2, 0, sqrt(2), param::BIN_SIZE);
}

pmp::Scalar Descriptor::compactness(pmp::SurfaceMesh& mesh)
{
    auto S = surface_area(mesh);
    auto V = volume(mesh);
    return (S * S * S) / (V * V * 36 * M_PI);
}

} // namespace mmr