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
    std::filesystem::path p = name_;
    name = p.replace_extension().string();
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

void Histogram::save()
{
    std::ofstream fout;
    fout.open(util::getExportDir("histogram/data/") + name + ".txt");
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
    float sum = 0;
    for (unsigned int i = 0; i < histogram.size(); i++)
            sum += histogram[i];

    for (unsigned int i = 0; i < histogram.size(); i++)
        histogram[i] /= sum;
}

// DESCRIPTOR ================================================================================
// ===========================================================================================

void Descriptor::histograms(Database& db)
{
    for (size_t i = 0; i < db.m_entries.size(); i++)
    {
        Entry& entry = db.m_entries[i];
        A3(entry).save();
        D1(entry).save();
        D2(entry).save();
    }
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

pmp::Scalar Descriptor::compactness(pmp::SurfaceMesh& mesh)
{
    auto S = surface_area(mesh);
    auto V = volume(mesh);
    return (S * S * S) / (V * V * 36 * M_PI);
}

Histogram Descriptor::A3(Entry& entry)
{
    auto& mesh = entry.mesh;
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh.n_vertices() - 1),
                  std::mt19937(seed));

    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    // Use pointers to create create data on heap
    std::vector<float>* angles = new std::vector<float>();
    angles->reserve(param::TARGET_VALUE);

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

        angles->push_back(angle);
    }

    std::string name = Entry::toString(entry.statistics["filename"]);
    return Histogram(name + "_A3", *angles, 0, param::A3_MAX_VALUE, param::BIN_SIZE);
}

Histogram Descriptor::D1(Entry& entry)
{
    auto& mesh = entry.mesh;

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
        float d = distance(center, points[pmp::Vertex(v)]);
        D1->push_back(d);
    }
    std::string name = Entry::toString(entry.statistics["filename"]);
    return Histogram(name + "_D1", *D1, 0, param::D1_MAX_VALUE, param::BIN_SIZE);
}

Histogram Descriptor::D2(Entry& entry)
{
    auto& mesh = entry.mesh;

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
        float d = distance(p1, p2);
        D2->push_back(d);
    }
    std::string name = Entry::toString(entry.statistics["filename"]);
    return Histogram(name + "_D2", *D2, 0, param::D2_MAX_VALUE, param::BIN_SIZE);
}
} // namespace mmr