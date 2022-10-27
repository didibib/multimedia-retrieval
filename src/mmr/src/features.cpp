#include "features.h"

using pmp::Scalar;

namespace mmr {

Scalar FeatureVector::distance(Histogram& h1, Histogram& h2)
{
    std::vector<float>::size_type N1 = h1.histogram.size();
    std::vector<float>::size_type N2 = h2.histogram.size();
    feature_t *f1 = new feature_t[N1], *f2 = new feature_t[N2];
    Scalar *w1 = new Scalar[N1], *w2 = new Scalar[N2];
    int i = 0;
    for (std::vector<float>::iterator iter = h1.histogram.begin();
         iter != h1.histogram.end(); ++iter, ++i)
    {
        f1[i] = i;
        w1[i] = h1.histogram[*iter];
    }
    i = 0;
    for (std::vector<float>::iterator iter = h1.histogram.begin();
         iter != h2.histogram.end(); ++iter, ++i)
    {
        f2[i] = i;
        w2[i] = h2.histogram[*iter];
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
    for (auto &i : index)
    {
        auto iter = data1.find(i);
        if (iter == data1.end() || iter == data2.end())
            continue;
        f1 << f1, Scalar(data1[i]);
        f2 << f2, Scalar(data2[i]);
    }
    return (f1 - f2).norm();
}
} // namespace mmr
