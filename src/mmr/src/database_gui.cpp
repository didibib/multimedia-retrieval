#include "database_gui.h"
#include "util.h"
#include "descriptors.h"
#include <imgui.h>
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

    if (ImGui::MenuItem("Normalize and Remesh Query"))
    {
        for (auto& q : db.m_queries)
        {
            Normalize::all_steps(q.getMesh());
            Normalize::remesh(q.getMesh());
            q.isNormalized = true;
        }
    }

     if (ImGui::MenuItem("Print kNN list of every shapes"))
    {
        int qi = 0;
        for (auto& q : db.m_entries)
        {
            qi++;
            int ei = 0;
            if (!q.isNormalized)
            {
                Normalize::all_steps(q.getMesh());
                Normalize::remesh(q.getMesh());
                q.isNormalized = true;
            }
            
            auto mi = Database::ANN(6, 0, q, db);
            std::vector<int> e = mi["knn"];
            std::cout
                << "nearest neighbour of" << std::setw(4) << qi << std::setw(10)
                << FeatureVector::toString(q.features["label"]) << " is"
                << std::setw(4) << e[1] + 1 << std::setw(10)
                << FeatureVector::toString(db.m_entries[e[1]].features["label"])
                << std::setw(4) << e[2] + 1 << std::setw(10)
                << FeatureVector::toString(db.m_entries[e[2]].features["label"])
                << std::setw(4) << e[3] + 1 << std::setw(10)
                << FeatureVector::toString(db.m_entries[e[3]].features["label"])
                << std::setw(4) << e[4] + 1 << std::setw(10)
                << FeatureVector::toString(db.m_entries[e[4]].features["label"])
                << std::setw(4) << e[5] + 1 << std::setw(10)
                << FeatureVector::toString(db.m_entries[e[5]].features["label"])
                << std::endl;
            /*for (auto& e : db.m_entries)
            {
                ei++;
                printf("Distance function between %i and %i is %f\n", qi, ei,
                       mmr::FeatureVector::distance(
                           q.features.histograms, e.features.histograms,
                           q.features.features, e.features.features));
                
            }*/
            
        }
     }
    if (ImGui::MenuItem("Print Accuracy"))
    {
        
        float globalScore = 0.0f;
        for (int i = 0; i < db.m_entries.size(); i++)
        {
            printf("Entry: %i\n", i);
            float score = 0;
            std::vector<float> distances;
            for (int j = 0; j < db.m_entries.size(); j++)
            {
                if (i == j)
                    continue;
                distances.push_back(mmr::FeatureVector::distance(
                    db.m_entries[i].features.features,
                    db.m_entries[j].features.features));
            }
            std::vector<int> kIndices = mmr::FeatureVector::kMeansIndices(
                i, distances, db.m_entries.size());

            // helpful struct to find the most occured label
            struct decisionClass
            {
                std::vector<::std::string> labels;
                std::vector<int> counts;
                int end;
            } classes;
            classes.end = 0;

            // search through the helpful label container and increment the occurance
            for (int k = 0; k < kIndices.size(); k++)
            {
                bool isFound = false;
                for (int c = 0; c < classes.end; c++)
                {
                    if (!classes.labels[c].compare(db.m_labels[kIndices[k]]))
                    {
                        classes.counts[c]++;
                        isFound = true;
                    }
                    if (isFound)
                        break;
                }

                // add the label if not found
                if (!isFound)
                {
                    classes.labels.push_back(db.m_labels[kIndices[k]]);
                    classes.counts.push_back(1);
                    classes.end++;
                }
            }
            int max = 0;
            int cIndex;
            for (int c = 0; c < classes.end; c++)
            {
                if (classes.counts[c] > max)
                {
                    max = classes.counts[c];
                    cIndex = c;
                }
            }

            if (!db.m_labels[i].compare(classes.labels[cIndex]))
            {
                score++;
            }
            globalScore += score;
            //printf("Score is %f\n", score / (float)kIndices.size());

        }
        printf("Final accuracy is: %f\n",
               100.0f * globalScore / (float)db.m_entries.size());     
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

        if (ImGui::MenuItem("View"))
        {
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