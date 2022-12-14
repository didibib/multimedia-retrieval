
#include <pmp/algorithms/DifferentialGeometry.h>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>

#include "descriptors.h"
#include "database.h"
#include "settings.h"
#include "util.h"
#include "entry.h"

namespace mmr {
Histogram::Histogram(Entry* entry, std::string descriptor,
                     std::vector<float>& values, float min_value,
                     float max_value, int num_bins)
    : m_entry(entry)
{
    m_filename =
        Feature::toString(entry->fv["filename"]) + "_" + descriptor;
    m_descriptor = descriptor;
    m_minValue = min_value;
    m_maxValue = max_value;
    m_numBins = num_bins;
    m_binWidth = static_cast<float>(m_maxValue - m_minValue) / (m_numBins - 1);

    m_values.resize(m_numBins);
    m_bins.resize(m_numBins);

    for (size_t i = 0; i < m_numBins; i++)
        m_bins[i] = i * m_binWidth;

    create(values);
    normalize();
}

void Histogram::save()
{
    std::string label = Feature::toString(m_entry->fv["label"]);
    std::ofstream fout;
    fout.open(util::getExportDir("histogram/data/" + m_entry->db_name + "/" +
                                 label + "/" + m_descriptor + "/") +
              m_filename + ".txt");
    //
    // !! If you add data or change the order of lines, you also need update the python file !!
    //
    /*[0]*/ fout << m_filename << "\n";
    /*[1]*/ fout << label << "\n";
    /*[2]*/ fout << m_descriptor << "\n";
    /*[3]*/ fout << m_minValue << "\n";
    /*[4]*/ fout << m_maxValue << "\n";
    /*[5]*/ fout << m_binWidth << "\n";

    for (size_t i = 0; i < m_bins.size(); i++)
        /*[6]*/ fout << m_bins[i] << " ";

    fout << "\n";

    for (size_t i = 0; i < m_values.size(); i++)
        /*[7]*/ fout << m_values[i] << " ";

    fout.close();
}

void Histogram::serialize(std::string dir)
{
    std::string path = dir + "/" + m_filename + ".hi";
    std::ofstream file;
    file.open(path);
    
    file << m_descriptor << "\n";

    for (unsigned int i = 0; i < m_bins.size(); i++)
        file << m_bins[i] << Feature::CSV_DELIM;
    file << "\n";
    for (unsigned int i = 0; i < m_values.size(); i++)
        file << m_values[i] << Feature::CSV_DELIM;
    file.close();
}

void Histogram::exportTsneFormat(std::ofstream& file) const
{
    for (unsigned int i = 0; i < m_values.size(); i++)
        file << m_values[i] << Feature::SPACE;
}

std::string Histogram::deserialize(std::string path)
{
    m_bins.clear();
    m_values.clear();
    m_bins.resize(param::BIN_SIZE);
    m_values.resize(param::BIN_SIZE);

    std::ifstream file;
    file.open(path);

    std::string bins, values;

    std::getline(file, m_descriptor);
    std::getline(file, bins);
    std::getline(file, values);

    file.close();

    std::string bin, val;
    std::stringstream bins_ss(bins);
    std::stringstream values_ss(values);

    for (size_t i = 0; i < param::BIN_SIZE; i++)
    {
        std::getline(bins_ss, bin, ',');
        m_bins[i] = std::stof(bin);

        std::getline(values_ss, val, ',');
        m_values[i] = std::stof(val);
    }
    return m_descriptor;
}

void Histogram::create(std::vector<float>& values)
{
    for (unsigned int i = 0; i < values.size(); i++)
    {
        auto idx = std::floorf((values[i] - m_minValue) / m_binWidth);
        int index = static_cast<int>(idx);
        m_values[index]++;
    }
}

void Histogram::normalize()
{
    float sum = 0;
    for (unsigned int i = 0; i < m_values.size(); i++)
        sum += m_values[i];

    for (unsigned int i = 0; i < m_values.size(); i++)
        m_values[i] /= sum;
}

// DESCRIPTOR ================================================================================
// ===========================================================================================

void Descriptor::histograms(Database* db)
{
    printf("Creating histograms...\n");
    for (size_t i = 0; i < db->m_entries.size(); i++)
    {
        Entry& entry = db->m_entries[i];
        Histogram h = A3(&entry);
        h.save();
        entry.fv.addHistogram(h);

        h = D1(&entry);
        h.save();
        entry.fv.addHistogram(h);

        h = D2(&entry);
        h.save();
        entry.fv.addHistogram(h);

        h = D3(&entry);
        h.save();
        entry.fv.addHistogram(h);

        h = D4(&entry);
        h.save();
        entry.fv.addHistogram(h);
    }
    printf("Histograms saved!\n");
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
    return static_cast<pmp::Scalar>((S * S * S) / (V * V * 36 * M_PI));
}

Histogram Descriptor::A3(Entry* entry)
{
    auto& mesh = entry->getMesh();
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

        float angle = std::acosf(dot / (u_mag * v_mag));
        if (isinf(angle) || isnan(angle))
        {
            i--;
            continue;
        }

        angles->push_back(angle * param::RAD_TO_DEG);
    }

    return Histogram(entry, "A3", *angles, 0, param::A3_MAX_VALUE,
                     param::BIN_SIZE);
}

Histogram Descriptor::D1(Entry* entry)
{
    auto& mesh = entry->getMesh();

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
    return Histogram(entry, "D1", *D1, 0, param::D1_MAX_VALUE, param::BIN_SIZE);
}

Histogram Descriptor::D2(Entry* entry)
{
    auto& mesh = entry->getMesh();

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
    return Histogram(entry, "D2", *D2, 0, param::D2_MAX_VALUE, param::BIN_SIZE);
}

Histogram Descriptor::D3(Entry* entry)
{
    auto& mesh = entry->getMesh();
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    std::vector<Scalar>* D3 = new std::vector<float>();
    D3->reserve(param::TARGET_VALUE);

    for (size_t i = 0; i < param::TARGET_VALUE; i++)
    {
        size_t r1 = random();

        size_t r2 = random();
        while (r1 == r2)
            r2 = random();

        size_t r3 = random();
        while (r3 == r1 || r3 == r2)
            r3 = random();

        Point v1 = points[Vertex(r1)];
        Point v2 = points[Vertex(r2)];
        Point v3 = points[Vertex(r3)];
        Point v1v2 = v2 - v1;
        Point v1v3 = v3 - v1;
        float dist12 = norm(v1v2);
        float dist13 = norm(v1v3);
        float theta = acosf(dot(v1v2, v1v3) / (dist12 * dist13));

        float result = sqrt(0.5f * dist12 * dist13 * sinf(theta));
        if (isinf(result) || isnan(result))
        {
            i--;
            continue;
        }

        D3->push_back(result);
    }
    return Histogram(entry, "D3", *D3, 0.0f, param::D3_MAX_VALUE,
                     param::BIN_SIZE);
}

Histogram Descriptor::D4(Entry* entry)
{
    auto& mesh = entry->getMesh();
    size_t mesh_size(mesh.vertices_size());
    std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random =
        std::bind(std::uniform_int_distribution<int>(0, mesh_size - 1),
                  std::mt19937(seed));
    auto points = mesh.get_vertex_property<pmp::Point>("v:point");

    std::vector<Scalar>* D4 = new std::vector<float>();
    D4->reserve(param::TARGET_VALUE);

    for (size_t i = 0; i < param::TARGET_VALUE; i++)
    {
        size_t r1 = random();
        size_t r2 = random();
        while (r1 == r2)
            r2 = random();

        size_t r3 = random();
        while (r3 == r1 || r3 == r2)
            r3 = random();

        size_t r4 = random();
        while (r4 == r1 || r4 == r2 || r4 == r3)
            r4 = random();

        Point v1 = points[Vertex(r1)];
        Point v2 = points[Vertex(r2)];
        Point v3 = points[Vertex(r3)];
        Point v4 = points[Vertex(r4)];
        Eigen::Matrix4f verts;
        verts.row(0) << v1[0], v1[1], v1[2], 1;
        verts.row(1) << v2[0], v2[1], v2[2], 1;
        verts.row(2) << v3[0], v3[1], v3[2], 1;
        verts.row(3) << v4[0], v4[1], v4[2], 1;
        D4->push_back(cbrt(abs(verts.determinant()) / 6));
    }
    return Histogram(entry, "D4", *D4, 0.0f, param::D4_MAX_VALUE,
                     param::BIN_SIZE);
}
} // namespace mmr