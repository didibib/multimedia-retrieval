#include "database.h"
#include "descriptors.h"
#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
#include "..\include\entry.h"

namespace mmr {


// DATABASE ==================================================================================
// ===========================================================================================

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

void Database::import(const std::string& path_)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 0;
    int iterator = 0;
    int maxModels = 380;
    int nQueries = 0;
    /*std::mt19937::result_type seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto random = std::bind(std::uniform_int_distribution<int>(0, maxModels - 1),
                  std::mt19937(seed));*/
    std::filesystem::path p = path_;
    name = p.filename().string();
    std::vector<int> qIndices;
    /*for (int i = 0; i < nQueries; i++)
    {
        qIndices.push_back(random());
    }*/
    for (const auto& file_entry : recursive_directory_iterator(path_))
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
        iterator++;
        // Create entry
        Entry entry(filename, label, path, name);
        /*if (nModels++ > 0)
        {*/
            // Update global statistics
            m_avgVerts += entry.mesh.n_vertices();
            m_avgFaces += entry.mesh.n_faces();

            m_labels.push_back(label);
            m_entries.push_back(std::move(entry));
            std::cout << "Model: " << ++nModels << std::endl;
       /* }
        else
        {
            m_queries.push_back(std::move(entry));
        }*/
    }

    if ((nModels - nQueries) == 0 || nModels == 0)
        return;

    m_avgVerts /= (nModels - nQueries);
    m_avgFaces /= (nModels - nQueries);
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;

    m_imported = true;
    m_columns = m_entries[0].features.n_statistics();
}

void Database::clear()
{
    m_entries.clear();
}

void Database::exportStatistics(std::string suffix) const
{
    if (m_entries.size() == 0)
        return;

    std::string filename = "statistics";
    if (!suffix.empty())
        filename += "_" + suffix;

    std::ofstream statistics;
    statistics.open(util::getExportDir() + filename + ".csv");

    // Headers
    for (auto const& [key, val] : m_entries[0].features.statistics())
        statistics << key << ",";
    statistics << "\n";

    // Rows
    for (unsigned int i = 0; i < m_entries.size(); i++)    
        m_entries[i].features.exportStatistics(statistics);
    
    statistics.close();
}

void Database::exportMeshes(std::string extension, std::string folder)
{
    for (auto& e : m_entries)
        e.write(extension, folder);
}
} // namespace mmr