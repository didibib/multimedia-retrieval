#include "features.h"
#include <fstream>
#include <filesystem>

using pmp::Scalar;

namespace mmr {

const std::string Feature::INT = "int";
const std::string Feature::FLOAT = "float";
const std::string Feature::STRING = "string";
const std::string Feature::CSV_DELIM = ",";

void FeatureVector::updateFeatureVector()
{
    std::vector<std::string> index = {"area", "compactness", "rectangularity",
                                      "diameter", "eccentricity"};
    features.resize(index.size());
    int j = 0;
    for (auto i=index.begin();i!=index.end();++i)
    {
        auto iter = m_statistics.find(*i);
        if (iter != m_statistics.end())
        {
            features[j] = toFloat(iter->second);
            j++;
        }
    }
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

void FeatureVector::exportStatistics(std::ofstream& file) const
{
    // Columns
    for (auto const& [key, val] : m_statistics)
        file << Feature::toString(val) << CSV_DELIM;
    file << "\n";
}

void FeatureVector::serialize(std::string dir, std::string filename)
{
    std::string filepath = dir + "/" + filename + ".fv";

    std::ofstream file;
    file.open(filepath);
    for (auto const& [key, val] : m_statistics)
        file << key << CSV_DELIM << Feature::toSerialize(val) << "\n";
    file.close();

    for (auto& h : m_histograms)
        h.second.serialize(dir);
}

void FeatureVector::deserialize(std::string folder) {

    if (!std::filesystem::is_directory(folder))
        return;
    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        std::filesystem::path path = entry.path();
        std::string ext = path.extension().string();

        if (ext == ".fv")
        {
            deserialize_fv(path.string());
        }
        if (ext == ".hi")
        {
            Histogram h;
            std::string key = h.deserialize(path.string());
            m_histograms[key] = h;
        }
    }
    updateFeatureVector();
}

void FeatureVector::deserialize_fv(std::string path) {
    std::ifstream file;
    std::string line;

    file.open(path);
    while (getline(file, line))
    {
        std::string key, type, value;
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, type, ',');
        std::getline(ss, value, ',');

        if (type == "int")
            m_statistics[key] = std::stoi(value);
        else if (type == "float")
            m_statistics[key] = std::stof(value);
        else if (type == "string")
            m_statistics[key] = value;
    }
    file.close();
}
} // namespace mmr
