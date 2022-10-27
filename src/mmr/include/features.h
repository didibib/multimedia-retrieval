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
    struct AnyGet
    {
        std::string operator()(int value) { return std::to_string(value); }
        std::string operator()(float value) { return std::to_string(value); }
        std::string operator()(const std::string& value) { return value; }
        std::string operator()(const pmp::Point& value)
        {
            // https://stackoverflow.com/questions/566052/can-you-encode-cr-lf-in-into-csv-files
            std::string x = std::to_string(value[0]) + "\n";
            std::string y = std::to_string(value[1]) + "\n";
            std::string z = std::to_string(value[2]);
            return std::string("\"" + x + y + z + "\"");
        }
    };

    using AnyType = std::variant<int, float, std::string, pmp::Point>;

    static std::string toString(const AnyType& input)
    {
        return std::visit(AnyGet{}, input);
    }

    static float e_dist(feature_t* F1, feature_t* F2)
    {
        return fabs(* F1 - *F2);
    }
};

class FeatureVector : public Feature
{
    std::map<std::string, AnyType> m_statistics;

public:
    static pmp::Scalar distance(Histogram& h1, Histogram& h2);
    static pmp::Scalar distance(std::map<std::string, pmp::Scalar>& data1,
                                std::map<std::string, pmp::Scalar>& data2,
                                std::vector<std::string>& index);

    AnyType& operator[](std::string key) { return m_statistics[key]; }

    const int n_statistics() const { return m_statistics.size(); };
    const std::map<std::string, AnyType>& statistics() const
    {
        return m_statistics;
    };
};

} // namespace mmr
