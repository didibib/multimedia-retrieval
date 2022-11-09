#include "features.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <pmp/algorithms/DifferentialGeometry.h>

using pmp::Scalar;

namespace mmr {

const std::string Feature::INT = "int";
const std::string Feature::FLOAT = "float";
const std::string Feature::STRING = "string";
const std::string Feature::CSV_DELIM = ",";
const std::string Feature::SPACE = " ";

void FeatureVector::updateFeatureVector()
{
    std::vector<std::string> Findex = {"area", "sphericity", "rectangularity",
                                       "diameter", "eccentricity"};
    features.resize(Findex.size());
    int j = 0;
    for (auto i : Findex)
    {
        auto iter = m_statistics.find(i);
        if (iter != m_statistics.end())
        {
            features[j] = toFloat(iter->second);
            j++;
        }
    }
}

void FeatureVector::updateHistograms()
{
    std::vector<std::string> Hindex = {"A3", "D1", "D2", "D3", "D4"};
    Scalar dist(0.f);
    for (auto i : Hindex)
    {
        auto iter = m_histograms.find(i);
        if (iter != m_histograms.end())
            histograms.push_back(iter->second);
    }
}

void FeatureVector::updateAllFeatures()
{
    allfeatures.clear();
    allfeatures.insert(allfeatures.end(), features.begin(), features.end());
    for (auto i : histograms)
    {
        std::vector<float> temp = i.values();
        allfeatures.insert(allfeatures.end(), temp.begin(), temp.end());
    }
}

Scalar FeatureVector::distance(std::vector<Histogram> h1,
                               std::vector<Histogram> h2,
                               Eigen::VectorXf& featuresA,
                               Eigen::VectorXf& featuresB)
{
    Scalar dist(0.f);
    Eigen::VectorXf v = featuresA - featuresB;
    int n = h1.size();
    for (size_t i = 0; i < n; i++)
        dist += distance(h1[i], h2[i]);
    dist += v.norm();
    dist /= (n + 1);
    return dist;
}

float e_dist(feature_t* F1, feature_t* F2) //Distance of two histogram bars
{
    return fabs(*F1 - *F2);
}

Scalar FeatureVector::distance(Histogram& h1, Histogram& h2)
{
    std::vector<float> his1(h1.values());
    std::vector<float> his2(h2.values());
    float d(-1);

    std::vector<float>::size_type N1 = his1.size();
    std::vector<float>::size_type N2 = his2.size();
    feature_t *f1 = new feature_t[N1], *f2 = new feature_t[N2];
    Scalar *w1 = new Scalar[N1], *w2 = new Scalar[N2];

    int i = 0;
    for (auto iter1 = his1.begin(); iter1 != his1.end(); ++iter1, ++i)
    {
        f1[i] = i;
        w1[i] = *iter1;
    }
    i = 0;
    for (auto iter2 = his2.begin(); iter2 != his2.end(); ++iter2, ++i)
    {
        f2[i] = i;
        w2[i] = *iter2;
    }
    signature_t s1 = {N1, f1, w1}, s2 = {N2, f2, w2};
    d = emd(&s1, &s2, e_dist, 0, 0);
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

void FeatureVector::exportStatistics(std::ofstream& file) const
{
    for (auto const& [key, val] : m_statistics)
        file << Feature::toString(val) << CSV_DELIM;
    file << "\n";
}

void FeatureVector::exportTsneFormat(std::ofstream& data)
{
    for (auto val : features)
        data << std::to_string(val) << SPACE;
    for (auto& h : m_histograms)
        h.second.exportTsneFormat(data);
    data << "\n";
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

void FeatureVector::updateStatistics(pmp::SurfaceMesh mesh)
{
    m_statistics["n_vertices"] = static_cast<int>(mesh.n_vertices());
    m_statistics["n_faces"] = static_cast<int>(mesh.n_faces());
    m_statistics["face_type"] = checkFaceType(mesh);
    m_statistics["distance_to_origin"] =
        pmp::distance(pmp::centroid(mesh), pmp::vec3(0, 0, 0));

    pmp::BoundingBox bb = mesh.bounds();

    m_statistics["bb_distance"] = pmp::distance(bb.max(), bb.min());
    m_statistics["surface_area"] = pmp::surface_area(mesh);

    m_statistics["bb_volume"] =
        ((bb.max()[0] - bb.min()[0]) * (bb.max()[1] - bb.min()[1]) *
         (bb.max()[2] - bb.min()[2]));
    float rectangularity = (volume(mesh) / ((bb.max()[0] - bb.min()[0]) *
                                            (bb.max()[1] - bb.min()[1]) *
                                            (bb.max()[2] - bb.min()[2])));
    m_statistics["rectangularity"] = rectangularity;
    float area = pmp::surface_area(mesh);
    m_statistics["area"] = area;
    m_statistics["volume"] = pmp::volume(mesh);
    Scalar compactness = Descriptor::compactness(mesh);
    m_statistics["compactness"] = compactness;
    m_statistics["sphericity"] = (1 / compactness);
    float eccentricity = Descriptor::eccentricity(mesh);
    m_statistics["eccentricity"] = eccentricity;
    float diameter = Descriptor::diameter(mesh);
    m_statistics["diameter"] = diameter;

    features.resize(5);
    features << area, (float)compactness, rectangularity, diameter,
        eccentricity;
}

std::vector<std::string> FeatureVector::getHeaders()
{
    static std::vector<std::string> headers = {
        "filename",       "label",      "n_vertices",
        "n_faces",        "face_type",  "distance_to_origin",
        "bb_distance",    "bb_volume",  "surface_area",
        "rectangularity", "area",       "volume",
        "compactness",    "sphericity", "eccentricity",
        "diameter"};
    return headers;
}

bool FeatureVector::deserialize(std::string folder)
{
    if (!std::filesystem::is_directory(folder))
        return false;
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
    updateHistograms();
    updateAllFeatures();
    return true;
}

void FeatureVector::deserialize_fv(std::string path)
{
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
