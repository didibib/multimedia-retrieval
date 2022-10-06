#include "database.h"
#include "util.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <pmp/algorithms/DifferentialGeometry.h>

namespace mmr {
void Entry::updateStatistics()
{
    statistics["n_vertices"] = static_cast<int>(mesh.n_vertices());
    statistics["n_faces"] = static_cast<int>(mesh.n_faces());
    statistics["centroid"] = pmp::centroid(mesh);

    BoundingBox bb = mesh.bounds();
    statistics["bb_center"] = bb.center();
    statistics["bb_min"] = bb.min();
    statistics["bb_max"] = bb.max();
}

Database::Database(const std::string path)
{
    import(path);
}

Entry* Database::get(int index)
{
    if (index < 0 || index >= m_entries.size())
        return nullptr;

    return &m_entries[index];
}

void Database::import(const std::string& path)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 0;
    int maxModels = 5;
    for (const auto& file_entry : recursive_directory_iterator(path))
    {
        std::string path = file_entry.path().string();
        std::string filename = file_entry.path().filename().string();
        std::string extension = file_entry.path().extension().string();
        std::string parentPath = file_entry.path().parent_path().string();
        std::string label =
            parentPath.substr(parentPath.find_last_of("/\\") + 1);

        if (extension != ".off" && extension != ".ply")
            continue;

        if (nModels > maxModels)
            break;

        // Create entry
        Entry entry(filename, label, path);
        m_entries.push_back(entry);
        m_labels.insert(label);

        // Update global statistics
        m_avgVerts += entry.mesh.n_vertices();
        m_avgFaces += entry.mesh.n_faces();

        std::cout << "Model: " << nModels++ << std::endl;
    }
    m_avgVerts /= nModels;
    m_avgFaces /= nModels;
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;

    if (nModels == 0)
        return;

    m_imported = true;
    m_columns = m_entries[0].statistics.size();
}

void Database::clear()
{
    m_entries.clear();
}

void Database::exportStatistics(std::string suffix)
{
    if (m_entries.size() == 0)
        return;

    std::string filename = "statistics";
    if (!suffix.empty())
        filename += "_" + suffix;

    std::ofstream statistics;
    statistics.open(util::getExportDir() + filename + ".csv");

    // Fill headers
    Entry entry = m_entries[0];
    for (auto const& [key, val] : entry.statistics)
        statistics << key << ",";
    statistics << "\n";

    // Fill columns
    for (unsigned int i = 0; i < m_entries.size(); i++)
        for (auto const& [key, val] : m_entries[i].statistics)
            statistics << Entry::toString(val) << ",";
    statistics << "\n";

    statistics.close();
}

void Database::exportMeshes(std::string folder)
{
    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        const Entry& entry = m_entries[i];
        auto it = entry.statistics.find("id");
        if (it != entry.statistics.cend())
            entry.mesh.write(util::getExportDir() + folder +
                             std::any_cast<std::string>(it->second));
    }
}


} // namespace mmr