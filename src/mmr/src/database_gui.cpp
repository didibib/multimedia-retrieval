#include "database_gui.h"
#include "util.h"
#include <imgui.h>
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/algorithms/Subdivision.h>
#include <pmp/algorithms/Smoothing.h>
#include <pmp/algorithms/Triangulation.h>
#include "normalization.h"

namespace mmr {

void DbGui::window(Database& db)
{
    if (!DbGui::m_showStatistics)
        return;

    if (db.m_entries.empty())
        return;

    if (!ImGui::Begin("Statistics", &m_showStatistics,
                      ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::MenuItem("Normalize all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalization::all_steps(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished normalizing!\n");
        }

        if (ImGui::MenuItem("Remesh all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalization::remesh(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished remeshing!\n");
        }

        ImGui::EndMenuBar();
    }

    statisticsTable(db);

    ImGui::End();
}

void DbGui::statisticsTable(Database& db)
{
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg;
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
        ImGui::TableHeader((it->first).c_str());
    }

    // Fill table
    for (int row = 0; row < db.m_entries.size(); row++)
    {
        ImGui::TableNextRow();

        const auto& stats = db.m_entries[row].statistics;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            int col = Entry::columnIndex(it->first);
            ImGui::TableSetColumnIndex(col);
            ImGui::Selectable(Entry::toString(it->second).c_str(),
                              &db.m_columnSelected[col],
                              ImGuiSelectableFlags_SpanAllColumns);

            algorithmsPopup(db, row, col);
        }
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void DbGui::algorithmsPopup(Database& db, const int& index, const int& column)
{
    if (ImGui::BeginPopupContextItem())
    {
        Entry& entry = db.m_entries[index];

        if (ImGui::MenuItem("View"))
        {
            m_selectedEntry = index;
        }
        if (ImGui::MenuItem("Reload"))
        {
            entry.reload();
        }
        ImGui::Separator();
        normalizationMenu(entry);

        ImGui::Separator();

        if (ImGui::BeginMenu("Export"))
        {
            if (ImGui::MenuItem("As .off"))
                entry.write(".off");
            if (ImGui::MenuItem("As .ply"))
                entry.write(".ply");

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Right-click to...");
}

void DbGui::normalizationMenu(Entry& entry)
{
    if (ImGui::BeginMenu("Normalization"))
    {
        if (ImGui::MenuItem("All steps"))
        {
            Normalization::all_steps(entry.mesh);
            entry.updateStatistics();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Remesh"))
        {
            Normalization::remesh(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Translate"))
        {
            Normalization::translate(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("PCA Pose"))
        {
            Normalization::pca_pose(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Flip"))
        {
            Normalization::flip(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Scale"))
        {
            Normalization::scale(entry.mesh);
            entry.updateStatistics();
        }
        ImGui::EndMenu();
    }
}

void DbGui::histogram(Database& db)
{
    if (!DbGui::m_showHistogram)
        return;

    if (!ImGui::Begin("Histogram", &m_showHistogram))
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
    {
        db.import(util::getDataDir("LabeledDB_new"));
        m_showStatistics = true;
    }

    if (db.m_imported)
        if (ImGui::MenuItem("Clear"))
        {
            db.clear();
            db.m_imported = false;
            m_showStatistics = false;
        }

    if (!m_showStatistics && db.m_imported)
        if (ImGui::MenuItem("Statistics"))
            m_showStatistics = true;

    if (db.m_imported)
        if (ImGui::BeginMenu("Export..."))
        {
            if (ImGui::MenuItem("Statistics"))
                db.exportStatistics();
            if (ImGui::BeginMenu("Meshes"))
            {
                if (ImGui::MenuItem("As .off"))
                {
                    for (auto& entry : db.m_entries)
                    {
                        entry.write(".off");
                    }
                }
                if (ImGui::MenuItem("As .ply"))
                {
                    for (auto& entry : db.m_entries)
                    {
                        entry.write(".ply");
                    }
                    printf("Finished exporting!\n");
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

    ImGui::EndMenu();
}

} // namespace mmr