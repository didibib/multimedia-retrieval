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

    int maxModels = 1;
    int nQueries = 1;

    std::filesystem::path p = path_;
    name = p.filename().string();
    std::vector<int> qIndices;
    /*for (int i = 0; i < nQueries; i++)
    {
        qIndices.push_back(random());
    }*/
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

       /* if (nModels > maxModels)
            break;*/

        // Create entry
        Entry entry(filename, label, path, name);
        /*m_avgVerts += entry.getMesh().n_vertices();
        m_avgFaces += entry.getMesh().n_faces();*/

        m_labels.push_back(label);

        m_entries.push_back(std::move(entry));
        std::cout << "Model: " << ++nModels << std::endl;
    }

    if ((nModels - nQueries) == 0 || nModels == 0)
        return;

    m_avgVerts /= (nModels - nQueries);
    m_avgFaces /= (nModels - nQueries);
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;

    m_imported = true;
    m_columns = m_entries[0].features.n_statistics();
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
    for (auto const& [key, val] : m_entries[0].features.statistics())
        statistics << key << ",";
    statistics << "\n";

    // Rows
    for (unsigned int i = 0; i < m_entries.size(); i++)
        m_entries[i].features.exportStatistics(statistics);

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
        labels << Feature::toString(e.features["label"]) << "\n";
        names << Feature::toString(e.features["filename"]) << "\n";
        e.features.exportTsneFormat(data);
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

std::map<std::string, std::vector<int>> Database::ANN(int k, float R,
                                                      mmr::Entry& target,
                                                      mmr::Database& db)
{
    std::map<std::string, std::vector<int>> Idx;
    std::vector<int> kIdx(k), RIdx;

    auto entries(db.m_entries);
    auto query(target.features);

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
        readPt(iter1.features.allfeatures, dataPts[nPts]);
        nPts++;
    }

    readPt(query.allfeatures, queryPt);

    kdTree = new ANNkd_tree(dataPts, nPts, dim);

    kdTree->annkSearch(queryPt, k, nnIdx, kdists, eps);

    R *= R;
    int n = kdTree->annkFRSearch(queryPt, R, nPts, nnRIdx, Rdists, eps);

    RIdx.resize(n);
    if (k > 0)
        for (size_t i = 0; i < k; i++)
            kIdx[i] = nnIdx[i];

    if (R > 0)
        for (size_t i = 0; nnRIdx[i] != ANN_NULL_IDX && i < nPts; i++)
            RIdx[i] = nnRIdx[i];

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