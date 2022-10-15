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
    if (!m_showStatistics)
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
        exportMenu(db);

        if (ImGui::MenuItem("Normalize all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalize::all_steps(e.mesh);
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
                Normalize::remesh(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished remeshing!\n");
        }

        if (ImGui::MenuItem("Translate all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalize::translate(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished translating!\n");
        }

        if (ImGui::MenuItem("PCA Pose all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalize::pca_pose(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished PCA Posing !\n");
        }

        if (ImGui::MenuItem("Flip Moment all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalize::flip(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished flipping !\n");
        }

        if (ImGui::MenuItem("Scale all"))
        {
            int i = 0;
            for (auto& e : db.m_entries)
            {
                Normalize::scale(e.mesh);
                e.updateStatistics();
                printf("%i\n", i++);
            }
            printf("Finished scaling !\n");
        }

        ImGui::EndMenuBar();
    }

    statisticsTable(db);

    ImGui::End();
}

void DbGui::statisticsTable(Database& db)
{
    if (db.m_entries.size() == 0)
        return;

    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));

    if (!ImGui::BeginTable("Statistics", static_cast<int>(db.m_columns), flags))
        return;

    // Setup headers
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    const auto& stats = db.m_entries[0].statistics;
    for (auto it = stats.cbegin(); it != stats.cend(); ++it)
    {
        int index = columnIndex(it->first);
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
            int col = columnIndex(it->first);
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

int DbGui::columnIndex(std::string key)
{
    auto headers = Entry::getHeaders();
    for (int i = 0; i < headers.size(); i++)
    {
        if (headers[i] == key)
            return i;
    }
    return 0;
}

void DbGui::exportMenu(Database& db)
{
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
                    entry.write(".off", "Meshes");
                }
            }
            if (ImGui::MenuItem("As .ply"))
            {
                for (auto& entry : db.m_entries)
                {
                    entry.write(".ply", "Meshes");
                }
                printf("Finished exporting!\n");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
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
            Normalize::all_steps(entry.mesh);
            entry.updateStatistics();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Remesh"))
        {
            Normalize::remesh(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Translate"))
        {
            Normalize::translate(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("PCA Pose"))
        {
            Normalize::pca_pose(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Flip"))
        {
            Normalize::flip(entry.mesh);
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Scale"))
        {
            Normalize::scale(entry.mesh);
            entry.updateStatistics();
        }
        ImGui::EndMenu();
    }
}

void DbGui::beginMenu(Database& db)
{
    window(db);

    if (!ImGui::BeginMenu("Database"))
        return;

    if (ImGui::BeginMenu("Import"))
    {
        if (ImGui::MenuItem("LabeledDB_new"))
        {
            db.import(util::getDataDir("LabeledDB_new"));
            m_showStatistics = true;
        }
        if (ImGui::MenuItem("Normalized"))
        {
            db.import(util::getExportDir("normalized"));
            m_showStatistics = true;
        }
        ImGui::EndMenu();
    }

    if (db.m_imported)
    {
        if (ImGui::MenuItem("Clear"))
        {
            db.clear();
            db.m_imported = false;
            m_showStatistics = false;
        }

        if (ImGui::MenuItem("Statistics"))
            m_showStatistics = true;
    }

    ImGui::EndMenu();
}

} // namespace mmr