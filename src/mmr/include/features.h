#pragma once

#include <pmp/SurfaceMesh.h>
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

    static float e_dist(feature_t* F1, feature_t* F2)
    {
        return fabs(*F1 - *F2);
    }
};

class FeatureVector : public Feature
{
    std::map<std::string, AnyType> m_statistics;
    std::map<std::string, Histogram> m_histograms;

    void deserialize_fv(std::string path);

public:
    Eigen::VectorXf features;

    void updateFeatureVector();
    static pmp::Scalar distance(Histogram& h1, Histogram& h2);
    inline static pmp::Scalar distance(Eigen::VectorXf& featuresA,
                                       Eigen::VectorXf& featuresB)
    {
        Eigen::VectorXf v = featuresA - featuresB;
        return v.norm();
    }

    AnyType& operator[](std::string key) { return m_statistics[key]; }

    const size_t n_statistics() const { return m_statistics.size(); };
    const std::map<std::string, AnyType>& statistics() const
    {
        return m_statistics;
    };
    void addHistogram(Histogram h) { m_histograms[h.descriptor()] = h; }

    void exportStatistics(std::ofstream&) const;
    void serialize(std::string dir, std::string filename);
    void deserialize(std::string dir);
};

} // namespace mmr
