#include "database_gui.h"
#include "util.h"
#include "descriptors.h"
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/algorithms/Subdivision.h>
#include <pmp/algorithms/Smoothing.h>
#include <pmp/algorithms/Triangulation.h>
#include "normalization.h"
#include "entry.h"

namespace mmr {
void DbGui::beginGui(Database& db)
{
    window(db);

    if (ImGui::BeginMenu("Database"))
    {
        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("LabeledDB_new"))
            {
                db.import(util::getDataDir("LabeledDB_new"));
                m_showStatistics = true;
                k = db.knn_k;
                r = db.rnn_r;
                entries_size = db.m_entries.size();
            }
            if (ImGui::MenuItem("Normalized"))
            {
                db.import(util::getExportDir("normalized"));
                m_showStatistics = true;
                for (auto& e : db.m_entries)
                {
                    e.isNormalized = true;
                }
                k = db.knn_k;
                r = db.rnn_r;
                entries_size = db.m_entries.size();
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

    if (ImGui::MenuItem("Normalize and Remesh Query"))
    {
        for (auto& q : db.m_queries)
        {
            Normalize::all_steps(q.getMesh());
            Normalize::remesh(q.getMesh());
            q.isNormalized = true;
        }
    }

    if (ImGui::MenuItem("Set up K and R"))
    {
        k = db.knn_k;
        r = db.rnn_r;
        ImGui::OpenPopup("NNSetting");
    }

    if (ImGui::BeginPopupModal("NNSetting"))
    {
        ImGui::InputInt("K", &k);
        if (k >= entries_size)
            k = entries_size;
        ImGui::InputFloat("R", &r);
        if (ImGui::Button("SAVE", ImVec2(100, 0)))
        {
            db.knn_k = k;
            db.rnn_r = r;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("CANCEL", ImVec2(100, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginMenu("Print Accuracy"))
    {
        if (ImGui::MenuItem("ANN_KNN"))
        {
            db.scoring(mmr::Database::NNmethod::ANN_KNN);
            std::cout << "scoring end" << std::endl;
            m_scoring = true;
        }
        if (ImGui::MenuItem("ANN_RNN"))
        {
            // not prepared
            db.scoring(mmr::Database::NNmethod::ANN_RNN);
            std::cout << "scoring end" << std::endl;
            m_scoring = true;
        }
        if (ImGui::MenuItem("KNN_handmade"))
        {
            db.scoring(mmr::Database::NNmethod::KNN_HANDMADE);
            std::cout << "scoring end" << std::endl;
            m_scoring = true;
        }
        if (ImGui::MenuItem("RNN_handmade"))
        {
            db.scoring(mmr::Database::NNmethod::RNN_HANDMADE);
            std::cout << "scoring end" << std::endl;
            m_scoring = true;
        }

        ImGui::EndMenu();
    }

    if (m_scoring)
        ImGui::OpenPopup("Scoring");

    if (ImGui::BeginPopupModal("Scoring"))
    {
        m_scoring = false;
        ImGui::PushFont(font);
        ImGui::Text(db.scoring_result.c_str());
        ImGui::PopFont();
        if (ImGui::Button("CLOSE", ImVec2(275, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

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
        normalizeAll(db);

        if (ImGui::MenuItem("Histograms..."))
        {
            Descriptor::histograms(&db);
            printf("Finished creating histograms!\n");
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
    const auto& stats = db.m_entries[0].features.statistics();
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

        const auto& stats = db.m_entries[row].features.statistics();
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            int col = columnIndex(it->first);
            ImGui::TableSetColumnIndex(col);

            ImGui::Selectable(Feature::toString(it->second).c_str(),
                              &db.m_columnSelected[col],
                              ImGuiSelectableFlags_SpanAllColumns);

            rightClickEntry(db, row, col);
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
        if (ImGui::MenuItem("Statistics..."))
        {
            db.exportStatistics();
            printf("Finished exporting statistics!\n");
        }
        if (ImGui::BeginMenu("Meshes"))
        {
            if (ImGui::MenuItem("As .off"))
            {
                for (auto& entry : db.m_entries)
                    entry.writeMesh(".off", "Meshes");
                printf("Finished exporting meshes!\n");
            }
            if (ImGui::MenuItem("As .ply"))
            {
                for (auto& entry : db.m_entries)
                    entry.writeMesh(".ply", "Meshes");
                printf("Finished exporting meshes!\n");
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Serialize..."))
        {
            for (auto& e : db.m_entries)
                e.serialize();
            printf("Finished serializing!\n");
        }

        if (ImGui::MenuItem("TSNE Format..."))
        {
            db.exportTsneFormat();
            printf("Finished TSNE formatting!\n");
        }

        ImGui::EndMenu();
    }
}

void DbGui::rightClickEntry(Database& db, const int& index, const int& column)
{
    if (ImGui::BeginPopupContextItem())
    {
        Entry& entry = db.m_entries[index];

        if (ImGui::BeginMenu("Query"))
        {
            if (ImGui::MenuItem("ANN_KNN"))
            {
                m_selectedEntries =
                    db.query(index, Database::NNmethod::ANN_KNN);
                m_newSelectedEntry = true;
                m_querying = true;
            }
            if (ImGui::MenuItem("ANN_RNN"))
            {
                m_selectedEntries =
                    db.query(index, Database::NNmethod::ANN_RNN);
                m_newSelectedEntry = true;
                m_querying = true;
            }
            if (ImGui::MenuItem("KNN_HANDMADE"))
            {
                m_selectedEntries =
                    db.query(index, Database::NNmethod::KNN_HANDMADE);
                m_newSelectedEntry = true;
                m_querying = true;
            }
            if (ImGui::MenuItem("RNN_HANDMADE"))
            {
                m_selectedEntries =
                    db.query(index, Database::NNmethod::RNN_HANDMADE);
                m_newSelectedEntry = true;
                m_querying = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("View"))
        {
            if (m_querying)
            {
                m_selectedEntries.clear();
                m_selectedEntries.resize(m_maxEntries);
                m_querying = false;
            }

            static int i = 0;
            m_selectedEntries[i++] = index;
            if (i >= m_maxEntries)
                i = 0;
            m_newSelectedEntry = true;
        }
        if (ImGui::MenuItem("Reload"))
        {
            entry.reload();
        }
        ImGui::Separator();
        normalizeEntry(entry);

        ImGui::Separator();
        if (ImGui::BeginMenu("Export"))
        {
            if (ImGui::MenuItem("As .off"))
                entry.writeMesh(".off");
            if (ImGui::MenuItem("As .ply"))
                entry.writeMesh(".ply");

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Right-click to...");
}

void DbGui::normalizeEntry(Entry& entry)
{
    if (ImGui::BeginMenu("Normalization"))
    {
        if (ImGui::MenuItem("All steps"))
        {
            Normalize::all_steps(entry.getMesh());
            entry.updateStatistics();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Remesh"))
        {
            Normalize::remesh(entry.getMesh());
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Translate"))
        {
            Normalize::translate(entry.getMesh());
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("PCA Pose"))
        {
            Normalize::pca_pose(entry.getMesh());
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Flip"))
        {
            Normalize::flip(entry.getMesh());
            entry.updateStatistics();
        }
        if (ImGui::MenuItem("Scale"))
        {
            Normalize::scale(entry.getMesh());
            entry.updateStatistics();
        }

        ImGui::EndMenu();
    }
}

void DbGui::normalizeAll(Database& db)
{
    if (!ImGui::BeginMenu("Normalize"))
        return;

    if (ImGui::MenuItem("Normalize all"))
    {
        int i = 0;
        for (auto& e : db.m_entries)
        {
            Normalize::all_steps(e.getMesh());
            e.updateStatistics();
            e.isNormalized = true;
            printf("%i\n", i++);
        }

        int j = 0;
        for (auto& q : db.m_queries)
        {
            Normalize::all_steps(q.getMesh());
            q.isNormalized = true;
            printf("Query:  %i\n", j++);
        }
        printf("Finished normalizing!\n");
    }

    if (ImGui::MenuItem("Remesh all"))
    {
        int i = 0;
        for (auto& e : db.m_entries)
        {
            Normalize::remesh(e.getMesh());
            e.updateStatistics();
            printf("%i\n", i++);
        }

        int j = 0;
        for (auto& q : db.m_queries)
        {
            Normalize::remesh(q.getMesh());
            printf("Query:  %i\n", j++);
        }
        printf("Finished remeshing!\n");
    }

    if (ImGui::MenuItem("Translate all"))
    {
        int i = 0;
        for (auto& e : db.m_entries)
        {
            Normalize::translate(e.getMesh());
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
            Normalize::pca_pose(e.getMesh());
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
            Normalize::flip(e.getMesh());
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
            Normalize::scale(e.getMesh());
            e.updateStatistics();
            printf("%i\n", i++);
        }
        printf("Finished scaling !\n");
    }
    ImGui::EndMenu();
}

} // namespace mmr