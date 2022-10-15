#pragma once

#include "util.h"
#include "descriptors.h"
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

    std::string checkFaceType()
    {
        if (mesh.is_triangle_mesh())
        {
            return "tri";
        }
        if (mesh.is_quad_mesh())
        {
            return "quad";
        }
        return "tri/quad";
    }

public:
    using AnyType = std::variant<int, float, std::string, pmp::Point>;
    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyGet{}, input);
    }

    static std::vector<std::string> getHeaders()
    {
#define N_DB_HEADERS 14
        static std::vector<std::string> headers = {
            "filename",    "label",       "n_vertices",
            "n_faces",     "face_type",   "distance_to_origin",
            "bb_distance", "bb_volume",   "rectangularity",
            "area",        "volume",      "compactness",
            "sphericity",  "eccentricity"};
        return headers;
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
        statistics["distance_to_origin"] =
            pmp::distance(pmp::centroid(mesh), pmp::vec3(0, 0, 0));

        pmp::BoundingBox bb = mesh.bounds();
        statistics["bb_distance"] = pmp::distance(bb.max(), bb.min());
        statistics["surface_area"] = pmp::surface_area(mesh);
        statistics["bb_volume"] = ((bb.max()[0] - bb.min()[0]) *
                                  (bb.max()[1] - bb.min()[1]) *
                                  (bb.max()[2] - bb.min()[2]));
        statistics["rectangularity"] =
            (volume(mesh) /
             ((bb.max()[0] - bb.min()[0]) * (bb.max()[1] - bb.min()[1]) *
              (bb.max()[2] - bb.min()[2])));
        statistics["area"] = surface_area(mesh);
        statistics["volume"] = volume(mesh);
        Scalar compactness = Descriptor::compactness(mesh);
        statistics["compactness"] = compactness;
        statistics["sphericity"] = (1 / compactness);
        statistics["eccentricity"] = Descriptor::eccentricity(mesh);
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
