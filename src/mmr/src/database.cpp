#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <imgui.h>
#include "database.h"
#include "util.h"

namespace mmr {

Database::Database(const std::string path)
{
    retrieve(path);
}

void Database::draw(const pmp::mat4& projection_matrix,
                    const pmp::mat4& modelview_matrix,
                    const std::string& draw_mode)
{
    for (int i = 0; i < m_entries.size(); i++)
    {
        m_entries[i].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
    }
}

void Database::drawModel(int index, const pmp::mat4& projection_matrix,
                         const pmp::mat4& modelview_matrix,
                         const std::string& draw_mode)
{
    m_entries[index].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
}

void Database::retrieve(const std::string& path)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 1;
    int maxModels = 2;
    for (const auto& file_entry : recursive_directory_iterator(path))
    {
        std::string path = file_entry.path().u8string();
        std::string filename = file_entry.path().filename().u8string();
        std::string extension = file_entry.path().extension().u8string();
        std::string label = path.substr(path.find_last_of("/\\") + 1);

        if (extension != ".off")
            continue;

        if (nModels > maxModels)
            break;
        // Create entry
        Entry entry;
        entry.label = label;
        entry.fileName = filename;
        entry.mesh.read(path);
        entry.nVertices = entry.mesh.n_vertices();
        entry.nFaces = entry.mesh.n_faces();
        m_entries.push_back(entry);

        // Update statistics
        m_avgVerts += entry.mesh.vertices_size();
        m_avgFaces += entry.mesh.faces_size();

        std::cout << "Model: " << nModels++ << std::endl;
    }
    m_avgVerts /= maxModels;
    m_avgFaces /= maxModels;
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;
}

void Database::clear()
{
    m_entries.clear();
}

void Database::exportStatistics(std::string suffix)
{
    std::string filename = "statistics";
    if (!suffix.empty())
        filename += "_" + suffix;

    std::ofstream statistics;
    statistics.open(util::getExportDir() + filename + ".csv");

    statistics << "label,n_vertices,n_faces,"
               << "\n";
    using namespace std;

    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        Entry entry = m_entries[i];
        statistics << entry.label << "," << entry.nVertices << ","
                   << entry.nFaces << "\n";
    }
    statistics.close();
}

void Database::exportMeshes(std::string folder)
{
    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        const Entry& entry = m_entries[i];
        entry.mesh.write(folder + entry.fileName);
    }
}
} // namespace mmr