#pragma once

#include "database.h"
#include <pmp/SurfaceMesh.h>
#include <vector>

namespace mmr {
class Histogram
{
    void create(std::vector<float>& values);
    void normalize();
    float m_minValue = 0;
    float m_maxValue = 0;
    int m_numBins = 0;
    float m_binWidth = 0;
    std::vector<float> m_bins;

    std::string m_filename;
    std::string m_descriptor;
    Entry& m_entry;

public:
    Histogram(Entry& entry, std::string descriptor, std::vector<float>& values, float min_value, float max_value,
              int num_bins);
    void save();
    std::vector<float> histogram;

};

class Descriptor
{
public:
    static void histograms(Database& db);
    static pmp::Scalar eccentricity(pmp::SurfaceMesh& entry);
    static pmp::Scalar compactness(pmp::SurfaceMesh& entry);
    static pmp::Scalar diameter(pmp::SurfaceMesh& entry);
    static Histogram A3(Entry& entry);
    static Histogram D1(Entry& entry);
    static Histogram D2(Entry& entry);
    static Histogram D3(Entry& entry);
    static Histogram D4(Entry& entry);

};
} // namespace mmr