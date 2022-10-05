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
    if (index < 0 || index >= m_entries.size())
        return;
    m_entries[index].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
}

void Database::retrieve(const std::string& path)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 0;
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
        entry.mesh.read(path);
        entry.statistics["id"] = filename;
        entry.statistics["label"] = label;
        entry.statistics["n_vertices"] = entry.mesh.n_vertices();
        entry.statistics["n_faces"] = entry.mesh.n_faces();
        m_entries.push_back(entry);

        // Update global statistics
        m_avgVerts += entry.mesh.vertices_size();
        m_avgFaces += entry.mesh.faces_size();

        std::cout << "Model: " << nModels++ << std::endl;
    }
    m_avgVerts /= nModels;
    m_avgFaces /= nModels;
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
        for (auto const& [key, val] : entry.statistics)
        {
            statistics << std::any_cast<std::string>(val) << ",";
        }
        statistics << "\n";
    }
    statistics.close();
}

void Database::exportMeshes(std::string folder)
{
    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        const Entry& entry = m_entries[i];

        auto it = entry.statistics.find("id");
        if (it != entry.statistics.cend())
            entry.mesh.write(folder +std::any_cast<std::string>(it->second));        
    }
}

void Database::guiDataMenu()
{
    if (ImGui::Button("Import"))
    {
        retrieve(util::getDataDir("LabeledDB_new"));
        m_imported = true;
    }
    if (m_imported && ImGui::Button("Clear"))
    {
        clear();
        m_imported = false;
    }

    if (ImGui::BeginMenu("Export"))
    {
        if (ImGui::Button("Statistics"))
            exportStatistics();
        if (ImGui::Button("Meshes"))
            exportMeshes(util::getExportDir("Meshes/"));

        ImGui::EndMenu();
    }

    if (!m_viewStatistics && ImGui::Button("View"))
        m_viewStatistics = true;
}

void Database::guiStatistics()
{
    if (m_entries.empty())
        return;

    ImGui::Begin(
        "DatabaseWindow", &m_viewStatistics,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
        ImGuiTableFlags_ContextMenuInBody;

    int columns = m_entries[0].statistics.size();

    if (!ImGui::BeginTable("Statistics", columns, flags))
        return;

    for (int row = 0; row < m_entries.size(); row++)
    {
        ImGui::TableNextRow();

        const auto& stats = m_entries[0].statistics;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            ImGui::TableSetColumnIndex(it - stats.cbegin());
            ImGui::Text("Hello %d,%d", col, row);
        }

        for (int col = 0; col < columns; col++)
        {
        }
    }

    ImGui::EndTable();
    ImGui::End();
}

void Database::beginMenu()
{
    if (!ImGui::BeginMenu("Database"))
        return;
    guiDataMenu();
    guiStatistics();

    ImGui::EndMenu();
}

} // namespace mmr