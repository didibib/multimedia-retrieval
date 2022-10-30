#include "features.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

using pmp::Scalar;

namespace mmr {

const std::string Feature::INT = "int";
const std::string Feature::FLOAT = "float";
const std::string Feature::STRING = "string";
const std::string Feature::CSV_DELIM = ",";

void FeatureVector::updateFeatureVector()
{
    /*features.resize(5);
    float area = m_statistics["area"];
    features << m_statistics["area"], m_statistics["compactness"],
        m_statistics["rectangularity"], m_statistics["diameter"],
        m_statistics["eccentricity"];
   
    float x0 = features(0);
    float x1 = features(1);
    float x2 = features(2);
    float x3 = features(3);
    float x4 = features(4);
    */
}
Scalar FeatureVector::distance(Histogram& h1, Histogram& h2)
{
    std::vector<float>::size_type N1 = h1.values().size();
    std::vector<float>::size_type N2 = h2.values().size();
    feature_t *f1 = new feature_t[N1], *f2 = new feature_t[N2];
    Scalar *w1 = new Scalar[N1], *w2 = new Scalar[N2];
    int i = 0;
    for (std::vector<float>::iterator iter = h1.values().begin();
         iter != h1.values().end(); ++iter, ++i)
    {
        f1[i] = i;
        w1[i] = h1.values()[*iter];
    }
    i = 0;
    for (std::vector<float>::iterator iter = h1.values().begin();
         iter != h2.values().end(); ++iter, ++i)
    {
        f2[i] = i;
        w2[i] = h2.values()[*iter];
    }
    signature_t s1 = {N1, f1, w1}, s2 = {N2, f2, w2};
    Scalar d = emd(&s1, &s2, Feature::e_dist, 0, 0);
    return d;
}

Scalar FeatureVector::distance(std::map<std::string, Scalar>& data1,
                               std::map<std::string, Scalar>& data2,
                               std::vector<std::string>& index)
{
    Eigen::VectorXf f1, f2;
    for (auto& i : index)
    {
        auto iter = data1.find(i);
        if (iter == data1.end() || iter == data2.end())
            continue;
        f1 << f1, Scalar(data1[i]);
        f2 << f2, Scalar(data2[i]);
    }
    return (f1 - f2).norm();
}

std::vector<int> FeatureVector::kMeansIndices(int index,
                                              std::vector<float>& distances, int size)
{
    int const k = 5;
    std::vector<int> indices;
    for (int i = 0; i< size -1; i++)
    {
        indices.push_back(i);
    }
    std::stable_sort(
        indices.begin(), indices.end(),
        [&distances](int i1, int i2) { return distances[i1] < distances[i2]; });
    std::vector<int> kIndices;
    for (int i = 0; i < size - 1; i++)
    {
        indices[i] = indices[i] >= index ? indices[i] + 1: indices[i];
    }
    for (int i = 0; i < k; i++)
    {
        kIndices.push_back(indices[i]);
    }
    return kIndices;
}

void FeatureVector::exportStatistics(std::ofstream& file) const
{
    // Columns
    for (auto const& [key, val] : m_statistics)
        file << Feature::toString(val) << CSV_DELIM;
    file << "\n";
}

void FeatureVector::serialize(std::string folder, std::string filename)
{
    std::string path = folder + "/" + filename + ".fv";
    std::ofstream file;
    file.open(path);
    for (auto const& [key, val] : m_statistics)
        file << key << CSV_DELIM << Feature::toSerialize(val) << "\n";
    file.close();

    for (auto const& h : m_histograms)
        h.second.serialize(folder, filename + ".hi");
}

void FeatureVector::deserialize(std::string filepath) {
    std::filesystem::path p = filepath;
    std::string ext = p.extension().string();

    if (ext == ".fv")
    {
    
    }
    if (ext == ".hi")
    {
    
    }
}
} // namespace mmr
