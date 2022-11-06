#pragma once

#include <pmp/MatVec.h>
#include <any>
#include <set>
#include <ostream>
#include <optional>

#include "util.h"
#include "features.h"
#include "entry.h"
#include <ANN/ANN.h>

namespace mmr {

class Database
{
    friend class DbGui;
    friend class Descriptor;

public:
    Database() = default;
    Database(const std::string path);
    void import(const std::string& path);
    Entry* get(int index);
    /*void draw(int index, const pmp::mat4& projectionMatrix,
              const pmp::mat4& modelviewMatrix, const std::string& drawMode);*/
    void clear();

    size_t getDbSize() { return m_entries.size(); }
    size_t getAvgVerts() { return m_avgVerts; }
    size_t getAvgFaces() { return m_avgFaces; }
    std::string name;

    void scoring();

    static std::map<std::string, std::vector<int>> ANN(float R,
                                                       mmr::Entry& target,
                                                       mmr::Database& db);
    static std::vector<int> Database::KNN(int i, mmr::Entry& target,
                                          mmr::Database& db);


private:
    std::vector<Entry> m_entries;
    std::vector<Entry> m_queries;
    std::vector<std::string> m_labels;
    std::vector<std::string> m_unique_labels;

    enum NNmethod
    {
        ANN_KNN,
        ANN_RNN,
        KNN_HANDMADE
    } scoring_flag;

    size_t m_avgVerts = 0;
    size_t m_avgFaces = 0;

    void exportStatistics(std::string suffix = "") const;
    void exportTsneFormat();
    void exportMeshes(std::string extension, std::string folder);

    bool m_imported = false;
    // Sadly cannot make this dynamic, since vector<bool> is stored as bits.
    bool m_columnSelected[16] = {false};
    size_t m_columns = 0;
    static void readPt(std::vector<float>& features, ANNpoint p);
    static std::vector<int> kMeansIndices(int index, std::vector<float>& distances,
                                   int size);
};
} // namespace mmr
