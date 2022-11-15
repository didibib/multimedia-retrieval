
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include <pmp/algorithms/Subdivision.h>
#include <pmp/algorithms/Smoothing.h>
#include <pmp/algorithms/Triangulation.h>
#include <pmp/algorithms/Decimation.h>

#include "database_gui.h"
#include "util.h"
#include "settings.h"
#include "descriptors.h"
#include "normalization.h"
#include "entry.h"

namespace mmr {

void DbGui::beginGui(Database& db)
{
    window(db);
    results(db);

    if (ImGui::BeginMenu("Database"))
    {
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
                for (auto& e : db.m_entries)
                {
                    e.isNormalized = true;
                }
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

    KRmenu(db);
    //queryMenu(db);

    if (ImGui::BeginMenu("Standardize Features"))
    {
        float means[5];
        float standardDeviation[5];
        for ( int i =0; i < 5; i++)
        {
            // calculate mean
            means[i] = 0.0f;
            for (int e = 0; e < db.m_entries.size(); e++)
            {
                means[i] += db.m_entries[e].fv.allfeatures[i];
            }
            means[i] /= (float)db.m_entries.size(); 

            // calcualte standard deviation
            standardDeviation[i] = 0.0f;
            for (int e = 0; e < db.m_entries.size(); e++)
            {
                standardDeviation[i] +=
                    (db.m_entries[e].fv.allfeatures[i] - means[i]) *
                    (db.m_entries[e].fv.allfeatures[i] - means[i]);
            }
            standardDeviation[i] =
                sqrtf(standardDeviation[i] / (float)db.m_entries.size());

            for (int ei = 0; ei < db.m_entries.size(); ei++)
            {

                db.m_entries[ei].fv.allfeatures[i] =
                    (db.m_entries[ei].fv.allfeatures[i] - means[i]) /
                        (6.0f * standardDeviation[i]) +
                    0.5f;
            }
        }
        ImGui::EndMenu();
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
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));

    if (!ImGui::BeginTable("Statistics", static_cast<int>(db.m_columns) + 1,
                           flags))
        return;

    ImGui::TableSetupScrollFreeze(2, 0);

    // Setup headers
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    const auto& stats = db.m_entries[0].fv.statistics();
    for (auto it = stats.cbegin(); it != stats.cend(); ++it)
    {
        int index = columnIndex(it->first) + 1;
        ImGui::TableSetColumnIndex(index);
        ImGui::TableHeader((it->first).c_str());
    }

    // Fill table
    for (int row = 0; row < db.m_entries.size(); row++)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text(std::to_string(row).c_str());

        const auto& stats = db.m_entries[row].fv.statistics();
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        {
            int col = columnIndex(it->first) + 1;
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
    auto headers = FeatureVector::getHeaders();
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

void DbGui::results(Database& db)
{
    if (m_selectedEntries.empty())
        return;

    if (ImGui::Begin("Results"))
    {
        ImGui::PushFont(font);
        for (size_t i = 0; i < m_selectedEntries.size(); i++)
        {
            Entry& entry = db.m_entries[m_selectedEntries[i]];
            std::ostringstream text;
            text << i << ": " << Feature::toString(entry.fv["filename"]);
            ImGui::Text(text.str().c_str());
        }
        ImGui::PopFont();
        ImGui::End();
    }
}

void DbGui::queryMenu(Database& db)
{
    if (db.m_entries.size() == 0)
        return;

    if (ImGui::BeginMenu("Query"))
    {
        static int index = 0;
        ImGui::InputInt("Index ", &index);

        const char* items[] = {"ANN_KNN", "ANN_RNN", "KNN_HANDMADE",
                               "RNN_HANDMADE"};
        static int item = 0;
        ImGui::Combo("Method", &item, items, IM_ARRAYSIZE(items));

        if (ImGui::Button("Search..."))
        {
            makeNewSelection();
            switch (item)
            {
                case 0:
                {
                    m_selectedEntries =
                        db.query(index, Database::NNmethod::ANN_KNN);
                }
                break;
                case 1:
                {
                    m_selectedEntries =
                        db.query(index, Database::NNmethod::ANN_RNN);
                }
                break;
                case 2:
                {
                    m_selectedEntries =
                        db.query(index, Database::NNmethod::KNN_HANDMADE);
                }
                break;
                case 3:
                {
                    m_selectedEntries =
                        db.query(index, Database::NNmethod::RNN_HANDMADE);
                }
                break;
            }
        }
        ImGui::EndMenu();
    }
}


void DbGui::KRmenu(Database& db)
{
    if (db.m_entries.size() == 0)
        return;

    if (ImGui::BeginMenu("Set up K and R"))
    {
        ImGui::InputInt("K", &db.knn_k);
        if (db.knn_k >= db.m_entries.size())
            db.knn_k = db.m_entries.size();
        ImGui::InputFloat("R", &db.rnn_r);
        ImGui::EndMenu();
    }
}

void DbGui::makeQuery(Database& db, int index, Database::NNmethod method)
{
    makeNewSelection();
    auto result = db.query(index, method);
    m_selectedEntries.push_back(index);
    m_selectedEntries.insert(std::end(m_selectedEntries), std::begin(result),
                             std::end(result));
}

void DbGui::rightClickEntry(Database& db, const int& index, const int& column)
{
    if (ImGui::BeginPopupContextItem())
    {
        Entry& entry = db.m_entries[index];

        if (ImGui::BeginMenu("Query"))
        {
            if (ImGui::MenuItem("ANN_KNN"))
                makeQuery(db, index, Database::NNmethod::ANN_KNN);   
            if (ImGui::MenuItem("ANN_RNN"))
                makeQuery(db, index, Database::NNmethod::ANN_RNN);
            if (ImGui::MenuItem("KNN_HANDMADE"))
                makeQuery(db, index, Database::NNmethod::KNN_HANDMADE);
            if (ImGui::MenuItem("RNN_HANDMADE"))
                makeQuery(db, index, Database::NNmethod::RNN_HANDMADE);
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("View"))
        {
            makeNewSelection();
            m_selectedEntries.push_back(index);
        }
        if (ImGui::MenuItem("Reload"))
        {
            entry.reload();
        }
        ImGui::Separator();

        normalizeEntry(entry);

        ImGui::Separator();

        if (ImGui::MenuItem("Retrieve class"))
        {
            makeNewSelection();
            std::string label = Feature::toString(entry.fv["label"]);
            for (int i = 0; i < db.m_entries.size(); i++)
            {
                Entry& e = db.m_entries[i];
                std::string l = Feature::toString(e.fv["label"]);
                if (l == label)
                    m_selectedEntries.push_back(i);
            }
        }

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
        ImGui::Separator();
        if (ImGui::MenuItem("Decimate"))
        {
            auto d = pmp::Decimation(entry.getMesh());
            d.decimate(param::TARGET_VALUE);
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