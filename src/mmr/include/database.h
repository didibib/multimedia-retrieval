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
    Entry(std::string filename, std::string label, std::string path, std::string db);

#define N_DB_HEADERS 16
    static std::vector<std::string> getHeaders();

    void updateStatistics();

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
        std::string path = util::getExportDir(folder + "/" + label);

        mesh.write(path + "/" + filename.string());
    }

public:
    std::map<std::string, AnyType> statistics;
    pmp::SurfaceMeshGL mesh;
    std::string original_path;
    std::string db_name;
};

class Database
{
    friend class DbGui;
    friend class Descriptor;

public:
    Database() = default;
    Database(const std::string path);
    void import(const std::string& path);
    Entry* get(int index);
    /*void draw(int index, const pmp::mat4& projectionMatrix,
              const pmp::mat4& modelviewMatrix, const std::string& drawMode);*/
    void clear();

    size_t getDbSize() { return m_entries.size(); }
    size_t getAvgVerts() { return m_avgVerts; }
    size_t getAvgFaces() { return m_avgFaces; }
    std::string name;

private:
    std::vector<Entry> m_entries;
    std::set<std::string> m_labels;

    size_t m_avgVerts = 0;
    size_t m_avgFaces = 0;

    void exportStatistics(std::string suffix = "") const;
    void exportMeshes(std::string extension, std::string folder);

    bool m_imported = false;
    // Sadly cannot make this dynamic, since vector<bool> is stored as bits.
    bool m_columnSelected[N_DB_HEADERS] = {false};
    size_t m_columns = 0;
};
} // namespace mmr
