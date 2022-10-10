#pragma once

#include "util.h"
#include <pmp/visualization/SurfaceMeshGL.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/MatVec.h>
#include <map>
#include <any>
#include <set>
#include <variant>
#include <ostream>
#include <optional>
#include <filesystem>

// TODO: Seperatie GUI features from the database

namespace mmr {

struct Entry
{
    struct AnyGet
    {
        std::string operator()(int value) { return std::to_string(value); }
        std::string operator()(float value) { return std::to_string(value); }
        std::string operator()(const std::string& value) { return value; }
        std::string operator()(const pmp::Point& value)
        {
            // https://stackoverflow.com/questions/566052/can-you-encode-cr-lf-in-into-csv-files
            std::string x = std::to_string(value[0]) + "\n";
            std::string y = std::to_string(value[1]) + "\n";
            std::string z = std::to_string(value[2]);
            return std::string("\"" + x + y + z + "\"");
        }
    };

public:
    using AnyType = std::variant<int, float, std::string, pmp::Point>;
    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyGet{}, input);
    }

    static std::vector<std::string> getHeaders()
    {
#define N_DB_HEADERS 16
        static std::vector<std::string> headers = {
            "filename",         "label",    "n_vertices",   "n_faces",      "face_type",
            "centroid",         "bb_center","bb_min",       "bb_max",       "bb_volume",
            "rectangularity",   "area",     "volume",       "compactness",  "sphericity",
            "eccentricity"};
        return headers;
    }

    static int columnIndex(std::string key)
    {
        auto headers = getHeaders();
        for (int i = 0; i < headers.size(); i++)
        {
            if (headers[i] == key)
                return i;
        }
        return 0;
    }

    Entry(std::string filename, std::string label, std::string path)
    {
        original_path = path;
        mesh.read(path);
        statistics["filename"] = filename;
        statistics["label"] = label;
        updateStatistics();
    }

    void updateStatistics()
    {
        statistics["n_vertices"] = static_cast<int>(mesh.n_vertices());
        statistics["n_faces"] = static_cast<int>(mesh.n_faces());
        statistics["face_type"] = checkFaceType();
        statistics["centroid"] = pmp::centroid(mesh);

        pmp::BoundingBox bb = mesh.bounds();
        statistics["bb_center"] = bb.center();
        statistics["bb_min"] = bb.min();
        statistics["bb_max"] = bb.max();
        statistics["bb_volume"] = bb.size();
        statistics["rectangularity"] = (volume(mesh) / bb.size());
        statistics["area"] = surface_area(mesh);
        statistics["volume"] = volume(mesh);
        statistics["compactness"] = compactness();
        statistics["sphericity"] = (1 / compactness());
        statistics["eccentricity"] = eccentricity();
    }

    void reload()
    {
        mesh.read(original_path);
        updateStatistics();
    }

    const void write(std::string extension, std::string folder = "")
    {
        std::filesystem::path filename =
            Entry::toString(statistics["filename"]);
        filename.replace_extension(extension);

        std::string label = toString(statistics["label"]);
        std::string path = util::getExportDir(folder) + "/" + label;

        mesh.write(path + "/" + filename.string());
    }

    std::string checkFaceType()
    {
        std::string type = "";
        if (mesh.is_triangle_mesh())
        {
            type += "tri";
        }
        if (mesh.is_quad_mesh())
        {
            type += "quad";
        }

        return type;
    }

    Scalar eccentricity()
    {
        unsigned int n_vertices = mesh.n_vertices();
        Point center = centroid(mesh);
        Eigen::MatrixXf input(3, n_vertices);
        auto points = mesh.get_vertex_property<Point>("v:point");
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
        Scalar ecc = eig.eigenvalues()[minv] / eig.eigenvalues()[maxv];
        ecc *= ecc > 0 ? 1.f : -1.f;
        return ecc;
    }

    Scalar compactness()
    {
        auto S = surface_area(mesh);
        auto V = volume(mesh);
        Scalar comp = pow(S, 3) / (pow(V, 2) * 36 * M_PI);
        return comp;
    }

public:
    std::map<std::string, AnyType> statistics;
    pmp::SurfaceMeshGL mesh;
    std::string original_path;
};

class Database
{
    friend class DbGui;

public:
    Database() = default;
    Database(const std::string path);
    void import(const std::string& path);
    Entry* get(int index);
    /*void draw(int index, const pmp::mat4& projectionMatrix,
              const pmp::mat4& modelviewMatrix, const std::string& drawMode);*/
    void clear();

    int getDbSize() { return m_entries.size(); }
    int getAvgVerts() { return m_avgVerts; }
    int getAvgFaces() { return m_avgFaces; }
    int getLabelCount() { return m_avgFaces; }

private:
    std::vector<Entry> m_entries;
    std::set<std::string> m_labels;

    int m_avgVerts = 0;
    int m_avgFaces = 0;

    void exportStatistics(std::string suffix = "") const;
    void exportMeshes(std::string extension, std::string folder);

    bool m_imported = false;
    // Sadly cannot make this dynamic, since vector<bool> is stored as bits.
    bool m_columnSelected[N_DB_HEADERS] = {false};
    int m_columns = 0;
};
} // namespace mmr
