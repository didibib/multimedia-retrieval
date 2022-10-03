#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include "pmp/MatVec.h"

namespace mmr {
struct Entry
{
public:
    pmp::SurfaceMeshGL mesh;
    std::string label;
};

class Database
{
public:
    Database() = default;
    Database(const std::string path);
    void draw(const pmp::mat4& projection_matrix,
              const pmp::mat4& modelview_matrix,
              const std::string& draw_mode);
    void retrieve(const std::string& path);
    void clear();

private:
    static int avgVerts;
    static int avgFaces;
    std::vector<Entry> entries;
};
} // namespace mmr
