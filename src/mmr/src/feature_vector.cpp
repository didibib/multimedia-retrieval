#include "feature_vector.h"

using namespace pmp;

Scalar e_dist(feature_t *F1, feature_t *F2)
{
    return fabs(*F1 - *F2);
}

Scalar dist(mmr::Histogram *h1, mmr::Histogram *h2)
{
    std::vector<float>::size_type N1 = h1->histogram.size();
    std::vector<float>::size_type N2 = h2->histogram.size();
    feature_t *f1 = new feature_t[N1], *f2 = new feature_t[N2];
    Scalar *w1 = new Scalar[N1], *w2 = new Scalar[N2];
    int i = 0;
    for (auto it:h1->histogram)
    {
        f1[i] = i;
        w1[i] = it;
        ++i;
    }
    i = 0;
    for (auto it:h2->histogram)
    {
        f2[i] = i;
        w2[i] = it;
        ++i;
    }
    signature_t s1 = {N1, f1, w1}, s2 = {N2, f2, w2};
    Scalar distance = emd(&s1, &s2, e_dist, 0, 0);
    return distance;
}

Scalar dist(std::map<std::string, Scalar>* data1,
            std::map<std::string, Scalar>* data2, std::vector<std::string> *index)
{
    Scalar distance(0.f);
    for (auto &i : *index)
    {
        auto iter = data1->find(i);
        if (iter == data1->end() || iter == data2->end())
            continue;
        Scalar delta = (*data1)[i] - (*data2)[i];
        distance += delta * delta;
    }
    return sqrtf(distance);
}