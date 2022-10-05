#include "database.h"
#include "util.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/algorithms/Subdivision.h>
#include <pmp/algorithms/Smoothing.h>
#include <pmp/algorithms/Triangulation.h>

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

void Database::draw(const pmp::mat4& projection_matrix,
                    const pmp::mat4& modelview_matrix,
                    const std::string& draw_mode)
{
    for (int i = 0; i < m_entries.size(); i++)
        m_entries[i].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
}

void Database::drawModel(int index, const pmp::mat4& projection_matrix,
                         const pmp::mat4& modelview_matrix,
                         const std::string& draw_mode)
{
    if (index < 0 || index >= m_entries.size())
        return;
    m_entries[index].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
}

void Database::import(const std::string& path)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 0;
    int maxModels = 2;
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

        // Update global statistics
        m_avgVerts += entry.mesh.vertices_size();
        m_avgFaces += entry.mesh.faces_size();

        std::cout << "Model: " << nModels++ << std::endl;
    }
    m_avgVerts /= nModels;
    m_avgFaces /= nModels;
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;

    if (nModels == 0)
        return;

    m_imported = true;
    m_showStatistics = true;
    m_columns = m_entries[0].statistics.size();
    m_columnSelected.clear();
    m_columnSelected.reserve(m_columns);
    for (int i = 0; i < 10; ++i)
        m_columnSelected.emplace_back(new bool(false));
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

    // Fill columns
    for (unsigned int i = 0; i < m_entries.size(); i++)
        for (auto const& [key, val] : m_entries[i].statistics)
            statistics << Entry::to_string(val) << ",";
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
            entry.mesh.write(folder + std::any_cast<std::string>(it->second));
    }
}

void Database::guiBeginMenu()
{
    guiStatistics();
    guiHistogram();

    if (!ImGui::BeginMenu("Database"))
        return;
    guiDataMenu();

    ImGui::EndMenu();
}

void Database::guiDataMenu()
{
    if (ImGui::MenuItem("Import"))
        import(util::getDataDir("LabeledDB_new"));

    if (m_imported)
        if (ImGui::MenuItem("Clear"))
        {
            clear();
            m_imported = false;
        }
    if (!m_showStatistics && m_imported)
        if (ImGui::MenuItem("View"))
            m_showStatistics = true;

    if (m_imported)
        if (ImGui::BeginMenu("Export..."))
        {
            if (ImGui::MenuItem("Statistics"))
                exportStatistics();
            if (ImGui::MenuItem("Meshes"))
                exportMeshes(util::getExportDir("Meshes/"));

            ImGui::EndMenu();
        }
}

void Database::guiStatistics()
{
    if (m_entries.empty())
        return;

    if (!m_showStatistics)
        return;

    if (!ImGui::Begin("Statistics", &m_showStatistics))
    {
        ImGui::End();
        return;
    }
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody |
        ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));

    if (!ImGui::BeginTable("Statistics", m_columns, flags))
        return;

    // Setup headers
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    const auto& stats = m_entries[0].statistics;
    for (auto it = stats.cbegin(); it != stats.cend(); ++it)
    {
        int index = Entry::column_index(it->first);
        ImGui::TableSetColumnIndex(index);
        ImGui::PushID(index);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox("##checkall", m_columnSelected[index]);
        ImGui::PopStyleVar();
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TableHeader((it->first).c_str());
        ImGui::PopID();
    }

    // Fill table
    for (int row = 0; row < m_entries.size(); row++)
    {
        ImGui::TableNextRow();
        const auto& stats = m_entries[row].statistics;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            int index = Entry::column_index(it->first);
            ImGui::TableSetColumnIndex(index);
            ImGui::Selectable(Entry::to_string(it->second).c_str(),
                              m_columnSelected[index]);
            if (index != 0)
                continue;

            guiAlgorithms(row);
        }
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
    ImGui::End();
}

void Database::guiHistogram()
{
    m_showHistogram = false;
    for (const auto& b : m_columnSelected)
    {
        if (!(*b))
            continue;

        m_showHistogram = true;
    }

    if (!ImGui::Begin("Histogram", &m_showHistogram))
    {
        ImGui::End();
        return;
    }

    static ImPlotHistogramFlags hist_flags = ImPlotHistogramFlags_Density;
    ImGui::CheckboxFlags("Cumulative", (unsigned int*)&hist_flags,
                         ImPlotHistogramFlags_Cumulative);

    if (ImPlot::BeginPlot("Statistics"))
    {
        ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
        //
        //ImPlot::PlotHistogram("")
        //
        ImPlot::EndPlot();
    }

    ImGui::End();
}

void Database::guiAlgorithms(const int& index)
{
    if (ImGui::BeginPopupContextItem())
    {
        Entry& entry = m_entries[index];

        if (ImGui::Button("Subdivision"))
        {
            pmp::Triangulation(entry.mesh).triangulate();
            pmp::Subdivision(entry.mesh).catmull_clark();
            entry.updateStatistics();
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Right-click to edit");
}

} // namespace mmr