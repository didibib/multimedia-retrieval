#include "features.h"

using namespace pmp;

namespace mmr {

Scalar FeatureVector::distance(Histogram& h1, Histogram& h2)
{
    Scalar w1[10], w2[10];
    std::copy(h1.histogram.begin(), h1.histogram.end(), w1);
    std::copy(h2.histogram.begin(), h2.histogram.end(), w2);
    feature_t f1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
              f2[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    signature_t s1 = {10, f1, w1}, s2 = {10, f2, w2};
    Scalar d = emd(&s1, &s2, Feature::e_dist, 0, 0);
    return d;
}

Scalar FeatureVector::distance(std::map<std::string, Scalar>& data1,
                               std::map<std::string, Scalar>& data2,
                               std::vector<std::string>& index)
{
    Scalar d(0.f);
    for (auto &i : index)
    {
        auto iter = data1.find(i);
        if (iter == data1.end() || iter == data2.end())
            continue;
        Scalar delta = data1[i] - data2[i];
        d += delta * delta;
    }
    return sqrtf(d);
}
} // namespace mmr
