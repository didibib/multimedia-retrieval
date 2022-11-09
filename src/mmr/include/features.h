#pragma once

#include <pmp/visualization/SurfaceMeshGL.h>
#include <vector>
#include <variant>
#include <map>
#include "descriptors.h"
#include "emd.h"

namespace mmr {

class Feature
{
public:
    struct AnyString
    {
        std::string operator()(int value) { return std::to_string(value); }
        std::string operator()(float value) { return std::to_string(value); }
        std::string operator()(const std::string& value) { return value; }
    };
    struct AnyFloat
    {
        float operator()(std::string value) { return std::stof(value); }
        float operator()(int value) { return float(value); }
        float operator()(float value) { return value; }
    };

    static const std::string INT;
    static const std::string FLOAT;
    static const std::string STRING;
    static const std::string CSV_DELIM;
    static const std::string SPACE;

    struct AnySerialize
    {
        std::string operator()(int value) { return INT + CSV_DELIM + std::to_string(value); }
        std::string operator()(float value)
        {
            return FLOAT + CSV_DELIM + std::to_string(value);
        }
        std::string operator()(const std::string& value)
        {
            return STRING + CSV_DELIM + value;
        }
    };

    using AnyType = std::variant<int, float, std::string>;

    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyString{}, input);
    }
    static std::string toSerialize(const AnyType& input)
    {
        return std::visit(AnySerialize{}, input);
    }
    static float toFloat(const AnyType& input)
    {
        return std::visit(AnyFloat{}, input);
    }
};

class FeatureVector : public Feature
{
    std::string checkFaceType(pmp::SurfaceMesh mesh)
    {
        if (mesh.is_triangle_mesh())
        {
            return "tri";
        }
        if (mesh.is_quad_mesh())
        {
            return "quad";
        }
        return "tri/quad";
    }

    std::map<std::string, AnyType> m_statistics;
    std::map<std::string, Histogram> m_histograms;

    static pmp::Scalar distance(Histogram& h1, Histogram& h2);

    void deserialize_fv(std::string path);

public:
    Eigen::VectorXf features;
    std::vector<Histogram> histograms;
    std::vector<float> allfeatures;

    static pmp::Scalar distance(std::map<std::string, pmp::Scalar>& data1,
                                std::map<std::string, pmp::Scalar>& data2,
                                std::vector<std::string>& index);

    inline static pmp::Scalar distance(Eigen::VectorXf& featuresA,
                                       Eigen::VectorXf& featuresB)
    {
        Eigen::VectorXf v = featuresA - featuresB;
        return v.norm();
    }

    static pmp::Scalar distance(std::vector<Histogram> h1,
                                std::vector<Histogram> h2,
                                Eigen::VectorXf& featuresA,
                                Eigen::VectorXf& featuresB);

    void updateHistograms();
    void updateAllFeatures();

    void updateFeatureVector();
    void updateStatistics(pmp::SurfaceMesh);


    AnyType& operator[](std::string key) { return m_statistics[key]; }

    const size_t n_statistics() const { return m_statistics.size(); };
    const std::map<std::string, AnyType>& statistics() const
    {
        return m_statistics;
    };
    void addHistogram(Histogram h) { m_histograms[h.descriptor()] = h; }

    void exportStatistics(std::ofstream&) const;
    void exportTsneFormat(std::ofstream& data);
    void serialize(std::string dir, std::string filename);
    bool deserialize(std::string dir);

    static std::vector<std::string> getHeaders();
};

} // namespace mmr
