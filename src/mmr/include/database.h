#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include "pmp/MatVec.h"
#include <map>
#include <any>
#include <set>
#include <variant>
#include <ostream>

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
            std::string x = std::to_string(value[0]) + "/";
            std::string y = std::to_string(value[1]) + "/";
            std::string z = std::to_string(value[2]);
            return std::string(x + y + z);
        }
    };

public:
    Entry(std::string id, std::string label, std::string path)
    {
        mesh.read(path);
        statistics["id"] = id;
        statistics["label"] = label;
        updateStatistics();
    }

    pmp::SurfaceMeshGL mesh;

    using AnyType = std::variant<int, float, std::string, pmp::Point>;
    std::unordered_map<std::string, AnyType> statistics;
    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyGet{}, input);
    }

    void updateStatistics();

    static int columnIndex(std::string key)
    {
        static std::map<std::string, int> order = {
            {"id", 0},      {"label", 1},    {"n_vertices", 2},
            {"n_faces", 3}, {"centroid", 4}, {"bb_center", 5},
            {"bb_min", 6},  {"bb_max", 7}};

        return order[key];
    }
};

class Database
{
public:
    Database() = default;
    Database(const std::string path);
    void import(const std::string& path);
    Entry* get(int index);
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
    bool m_columnSelected[8] = {false};

    int m_columns = 0;
    friend class DbGui;
};
} // namespace mmr
