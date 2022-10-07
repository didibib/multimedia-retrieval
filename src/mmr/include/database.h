#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include <pmp/algorithms/DifferentialGeometry.h>
#include "pmp/MatVec.h"
#include <map>
#include <any>
#include <set>
#include <variant>
#include <ostream>
#include <optional>
#include "util.h"

// TODO: Seperatie GUI features from the database

namespace mmr {

#define N_DB_HEADERS 8

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
    std::map<std::string, AnyType> statistics;
    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyGet{}, input);
    }

    static std::vector<std::string> getHeaders()
    {
        static std::vector<std::string> headers = {
            "id",       "label",     "n_vertices", "n_faces",
            "centroid", "bb_center", "bb_min",     "bb_max"};
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

    Entry(std::string id, std::string label, std::string path)
    {
        original_path = path;
        mesh.read(path);
        statistics["id"] = id;
        statistics["label"] = label;
        updateStatistics();
    }

    void updateStatistics()
    {
        statistics["n_vertices"] = static_cast<int>(mesh.n_vertices());
        statistics["n_faces"] = static_cast<int>(mesh.n_faces());
        statistics["centroid"] = pmp::centroid(mesh);

        pmp::BoundingBox bb = mesh.bounds();
        statistics["bb_center"] = bb.center();
        statistics["bb_min"] = bb.min();
        statistics["bb_max"] = bb.max();
    }

    void write(std::string folder = "")
    {
        std::string filename = Entry::toString(statistics["id"]);
        mesh.write(util::getExportDir(folder) + filename);
    }

public:
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

    void exportStatistics(std::string suffix = "");
    void exportMeshes(std::string folder);

    bool m_imported = false;
    // Sadly cannot make this dynamic, since vector<bool> is stored as bits.
    bool m_columnSelected[N_DB_HEADERS] = {false};
    int m_columns = 0;
};
} // namespace mmr
