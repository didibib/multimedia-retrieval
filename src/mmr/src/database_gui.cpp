#include "database_gui.h"
#include "util.h"
#include <imgui.h>
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/algorithms/Subdivision.h>
#include <pmp/algorithms/Smoothing.h>
#include <pmp/algorithms/Triangulation.h>

namespace mmr {

bool DbGui::m_showHistogram = false;

void DbGui::window(Database& db)
{
    if (!db.m_showStatistics)
        return;

    if (db.m_entries.empty())
        return;

    if (!ImGui::Begin("Statistics", &db.m_showStatistics,
                      ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::MenuItem("Histogram", nullptr))
        {
            DbGui::m_showHistogram = true;
        }
        ImGui::EndMenuBar();
    }

    statistics(db);
    //histogram(db);

    ImGui::End();
}

void DbGui::statistics(Database& db)
{
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody |
        ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));

    if (!ImGui::BeginTable("Statistics", db.m_columns, flags))
        return;

    // Setup headers
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    const auto& stats = db.m_entries[0].statistics;
    for (auto it = stats.cbegin(); it != stats.cend(); ++it)
    {
        int index = Entry::columnIndex(it->first);
        ImGui::TableSetColumnIndex(index);
        ImGui::PushID(index);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox("##checkall", db.m_columnSelected[index]);
        ImGui::PopStyleVar();
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TableHeader((it->first).c_str());
        ImGui::PopID();
    }

    // Fill table
    for (int row = 0; row < db.m_entries.size(); row++)
    {
        ImGui::TableNextRow();
        const auto& stats = db.m_entries[row].statistics;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            int index = Entry::columnIndex(it->first);
            ImGui::TableSetColumnIndex(index);
            ImGui::Selectable(Entry::toString(it->second).c_str(),
                              db.m_columnSelected[index]);
            algorithms(db, row, index);
        }
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void DbGui::algorithms(Database& db, const int& index, const int& column)
{
    if (ImGui::BeginPopupContextItem())
    {
        Entry& entry = db.m_entries[index];

        if (ImGui::Button("Subdivision"))
        {
            pmp::Subdivision(entry.mesh).quad_tri();
            entry.updateStatistics();
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Right-click to edit");
}

void DbGui::histogram(Database& db)
{
    if (!DbGui::m_showHistogram)
        return;

    for (const auto& b : db.m_columnSelected)
    {
        if (!(*b))
            continue;
    }

    if (!ImGui::Begin("Histogram", &DbGui::m_showHistogram))
    {
        ImGui::End();
        return;
    }

    static ImPlotHistogramFlags hist_flags = ImPlotHistogramFlags_Density;
    ImGui::CheckboxFlags("Cumulative", (unsigned int*)&hist_flags,
                         ImPlotHistogramFlags_Cumulative);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    static float rmin;
    ImGui::DragFloat2("##Range", &rmin, 0.1f, -3, 13);
    ImGui::SameLine();
    ImGui::CheckboxFlags("Exclude Outliers", (unsigned int*)&hist_flags,
                         ImPlotHistogramFlags_NoOutliers);

    if (ImPlot::BeginPlot("##Statistics"))
    {
        ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

        /*ImPlot::PlotHistogram();
        ImPlot::PlotLine();*/

        ImPlot::EndPlot();
    }

    ImGui::End();
}

void DbGui::beginMenu(Database& db)
{
    window(db);
    histogram(db);

    if (!ImGui::BeginMenu("Database"))
        return;

    if (ImGui::MenuItem("Import"))
        db.import(util::getDataDir("LabeledDB_new"));

    if (db.m_imported)
        if (ImGui::MenuItem("Clear"))
        {
            db.clear();
            db.m_imported = false;
        }
    if (!db.m_showStatistics && db.m_imported)
        if (ImGui::MenuItem("View"))
            db.m_showStatistics = true;

    if (db.m_imported)
        if (ImGui::BeginMenu("Export..."))
        {
            if (ImGui::MenuItem("Statistics"))
                db.exportStatistics();
            if (ImGui::MenuItem("Meshes"))
                db.exportMeshes(util::getExportDir("Meshes/"));

            ImGui::EndMenu();
        }

    ImGui::EndMenu();
}

} // namespace mmr