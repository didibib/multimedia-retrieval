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

#ifndef NDEBUG
    int maxModels = 5;
#endif // DEBUG

    int nQueries = 1;

    std::filesystem::path p = path_;
    name = p.filename().string();
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

#ifndef NDEBUG
        if (nModels > maxModels)
            break;
#endif // DEBUG

        // Create entry
        Entry entry(filename, label, path, name);

        m_labels.push_back(label);

        m_entries.push_back(std::move(entry));
        std::cout << "Model: " << ++nModels << std::endl;
    }

    if ((nModels - nQueries) == 0 || nModels == 0)
        return;

    m_imported = true;
    m_columns = m_entries[0].fv.n_statistics();
    assert(m_columns <= MAX_COLUMNS);
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
    for (auto const& [key, val] : m_entries[0].fv.statistics())
        statistics << key << ",";
    statistics << "\n";

    // Rows
    for (unsigned int i = 0; i < m_entries.size(); i++)
        m_entries[i].fv.exportStatistics(statistics);

    statistics.close();
}

void Database::exportTsneFormat()
{
    std::ofstream data;
    std::ofstream labels;
    std::ofstream names;

    data.open(util::getExportDir() + "tsne_data" + ".txt");
    labels.open(util::getExportDir() + "tsne_labels" + ".txt");
    names.open(util::getExportDir() + "tsne_names" + ".txt");
    for (auto& e : m_entries)
    {
        labels << Feature::toString(e.fv["label"]) << "\n";
        names << Feature::toString(e.fv["filename"]) << "\n";
        e.fv.exportTsneFormat(data);
    }

    data.close();
    labels.close();
    names.close();
}

void Database::exportMeshes(std::string extension, std::string folder)
{
    for (auto& e : m_entries)
        e.writeMesh(extension, folder);
}

// SCORE =====================================================================================
// ===========================================================================================

std::vector<int> Database::scoring(NNmethod scoring_flag)
{
    auto start = std::chrono::system_clock::now();
    float globalScore = 0.0f;
    // helpful struct to aggregiate label accuracy
    struct labelAccuracy
    {
        std::vector<::std::string> labels;
        std::vector<float> scores;
        std::vector<int> counts;
        int end = 0;

    } labelsAccuracy;

    std::ostringstream result;
    std::string method;
    switch (scoring_flag)
    {
        case NNmethod::ANN_KNN:
        {
            method = "ANN_KNN";
            result << std::left << std::setw(17) << method
                   << ": (k = " << this->knn_k << ")" << std::endl;
        }
        break;
        case NNmethod::ANN_RNN:
        {
            method = "ANN_RNN";        
            result << std::left << std::setw(17) << method
                   << ": (R = " << this->rnn_r << ")" << std::endl;
        }
        break;
        case NNmethod::KNN_HANDMADE:
        {
            method = "KNN_handmade";          
            result << std::left << std::setw(17) << method
                   << ": (k = " << this->knn_k << ")" << std::endl;
        }
        break;
        default:
            break;
    }

    std::vector<int> kIndices;
    for (int i = 0; i < m_entries.size(); i++)
    {
        float score = 0;
        kIndices = query(i, scoring_flag);

        // helpful struct to find the most occured label
        struct decisionClass
        {
            std::vector<::std::string> labels;
            std::vector<int> counts;
            int end = 0;
        } classes;

        // search through the helpful label container and increment the occurance
        for (int k = 0; k < kIndices.size(); k++)
        {
            bool isFound = false;
            for (int c = 0; c < classes.end; c++)
            {
                if (!classes.labels[c].compare(m_labels[kIndices[k]]))
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
                classes.labels.push_back(m_labels[kIndices[k]]);
                classes.counts.push_back(1);
                classes.end++;
            }
        }
        int max = 0;
        int cIndex = 0;
        for (int c = 0; c < classes.end; c++)
        {
            if (classes.counts[c] > max)
            {
                max = classes.counts[c];
                cIndex = c;
            }
        }

        bool isFound = false;
        int lIndex = 0;
        for (int l = 0; l < labelsAccuracy.end; l++)
        {
            if (!labelsAccuracy.labels[l].compare(m_labels[i]))
            {
                isFound = true;
                lIndex = l;
            }
            if (isFound)
                break;
        }
        if (!m_labels[i].compare(classes.labels[cIndex]))
        {
            score++;
            if (isFound)
            {
                labelsAccuracy.counts[lIndex]++;
                labelsAccuracy.scores[lIndex]++;
            }
            else
            {
                labelsAccuracy.counts.push_back(1);
                labelsAccuracy.scores.push_back(1);
                labelsAccuracy.labels.push_back(m_labels[i]);
                labelsAccuracy.end++;
            }
        }
        else
        {
            if (isFound)
            {
                labelsAccuracy.counts[lIndex]++;
            }
            else
            {
                labelsAccuracy.counts.push_back(1);
                labelsAccuracy.scores.push_back(0);
                labelsAccuracy.labels.push_back(m_labels[i]);
                labelsAccuracy.end++;
            }
        }
        globalScore += score;
        //printf("Score is %f\n", score / (float)kIndices.size());
    }

    std::ofstream result_labels;
    std::ofstream result_data;
    result_labels.open(util::getExportDir("results") + "result_labels_" + method +
                       ".txt");
    result_data.open(util::getExportDir("results") + "result_data_" + method +
                     ".txt");

    for (int cl = 0; cl < labelsAccuracy.labels.size(); cl++)
    {
        std::string label = labelsAccuracy.labels[cl];
        float data = 100.0f * labelsAccuracy.scores[cl] /
                     (float)labelsAccuracy.counts[cl];

        result_labels << label << "\n";
        result_data << data << "\n";

        result << "Class acurracy of: " << std::left << std::setw(10) << label
               << "is" << std::setw(7)
               << std::setiosflags(std::ios::fixed | std::ios::right)
               << std::setprecision(2) << data << std::endl;
    }
    result_labels.close();
    result_data.close();

    result << std::left << std::setw(29) << "Final accuracy   :"
           << "is" << std::setw(7)
           << std::setiosflags(std::ios::fixed | std::ios::right)
           << std::setprecision(2)
           << 100.0f * globalScore / (float)m_entries.size() << std::endl;

    auto end = std::chrono::system_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    result << std::left << std::setw(17) << "time used"
           << ": " << std::setw(10)
           << std::setiosflags(std::ios::fixed | std::ios::left)
           << std::setprecision(4)
           << double(duration.count()) *
                  std::chrono::microseconds::period::num /
                  std::chrono::microseconds::period::den
           << "seconds" << std::endl;
    this->scoring_result = result.str();
    return kIndices;
}

std::vector<int> Database::query(int i, Database::NNmethod method)
{
    switch (method)
    {
        case NNmethod::ANN_KNN:
            return Database::ANN(knn_k, 0, i, *this)["knn"];
        case NNmethod::ANN_RNN:
            return Database::ANN(0, rnn_r, i, *this)["rnn"];
        case NNmethod::KNN_HANDMADE:
            return Database::KNN(knn_k, i, *this);
        case NNmethod::RNN_HANDMADE:
            return Database::RNN(knn_k, i, *this);
    }
}

// KNN =======================================================================================
// ===========================================================================================

std::vector<int> Database::kMeansIndices(int k, int index,
                                         std::vector<float>& distances,
                                         int size)
{
    std::vector<int> indices;
    for (int i = 0; i < size - 1; i++)
    {
        indices.push_back(i);
    }
    std::stable_sort(
        indices.begin(), indices.end(),
        [&distances](int i1, int i2) { return distances[i1] < distances[i2]; });
    std::vector<int> kIndices;
    for (int i = 0; i < size - 1; i++)
    {
        indices[i] = indices[i] >= index ? indices[i] + 1 : indices[i];
    }
    for (int i = 0; i < k; i++)
    {
        kIndices.push_back(indices[i]);
    }
    return kIndices;
}

std::vector<int> Database::KNN(int k, int i, mmr::Database& db)
{
    auto entries(db.m_entries);
    std::vector<float> distances;
    for (int j = 0; j < db.m_entries.size(); j++)
    {
        if (i == j)
            continue;
        distances.push_back(mmr::FeatureVector::distance(
            entries[i].fv.histograms, entries[j].fv.histograms,
            entries[i].fv.features, entries[j].fv.features));
    }
    std::vector<int> kIndices = kMeansIndices(k, i, distances, entries.size());
    return kIndices;
}

std::vector<int> Database::RNN(int k, int i, mmr::Database& db)
{
    auto entries(db.m_entries);
    std::vector<float> distances;
    std::vector<int> kIndices;
    const float R = 1.0f;
    for (int j = 0; j < db.m_entries.size(); j++)
    {
        if (i == j)
            continue;
        float distance = mmr::FeatureVector::distance(
            entries[i].fv.histograms, entries[j].fv.histograms,
            entries[i].fv.features, entries[j].fv.features);
        if (distance <= R)
            kIndices.push_back(j);
    }
    return kIndices;
}

// ANN =======================================================================================
// ===========================================================================================

void Database::readPt(std::vector<float>& features, ANNpoint p)
{
    int j = 0;
    for (auto& i : features)
    {
        p[j] = i;
        j++;
    }
}

std::map<std::string, std::vector<int>> Database::ANN(int k, float R, int i,
                                                      mmr::Database& db)
{
    auto entries(db.m_entries);
    auto query(db.m_entries[i].fv);

    if (k)
        k += 1;

    std::map<std::string, std::vector<int>> Idx;
    std::vector<int> kIdx(k), RIdx;

    int dim(query.allfeatures.size());
    double eps(0);
    int maxPts(1000);

    int nPts(0);
    ANNpointArray dataPts;
    ANNpoint queryPt;
    ANNidxArray nnIdx;
    ANNidxArray nnRIdx;
    ANNdistArray kdists;
    ANNdistArray Rdists;
    ANNkd_tree* kdTree;

    queryPt = annAllocPt(dim);
    dataPts = annAllocPts(maxPts, dim);
    nnIdx = new ANNidx[k];
    nnRIdx = new ANNidx[maxPts];
    kdists = new ANNdist[k];
    Rdists = new ANNdist[maxPts];

    for (auto& iter1 : entries)
    {
        readPt(iter1.fv.allfeatures, dataPts[nPts]);
        nPts++;
    }

    readPt(query.allfeatures, queryPt);

    kdTree = new ANNkd_tree(dataPts, nPts, dim);

    if (k > 0)
    {
        kdTree->annkSearch(queryPt, k, nnIdx, kdists, eps);
        kIdx.resize(k - 1);
        for (size_t i = 1; i < k; i++)
            kIdx[i - 1] = nnIdx[i];
    }

    if (R > 0)
    {
        R *= R;
        int n = kdTree->annkFRSearch(queryPt, R, nPts, nnRIdx, Rdists, eps);
        RIdx.resize(n);
        for (size_t i = 1; nnRIdx[i] != ANN_NULL_IDX && i < nPts; i++)
            RIdx[i - 1] = nnRIdx[i];
    }

    delete[] nnIdx;
    delete[] nnRIdx;
    delete[] kdists;
    delete[] Rdists;
    delete kdTree;
    annClose();

    Idx["knn"] = kIdx;
    Idx["rnn"] = RIdx;

    return Idx;
}

} // namespace mmr