#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include "pmp/MatVec.h"

namespace mmr {
struct Entry
{
public:
    pmp::SurfaceMeshGL mesh;
    pmp::mat4 translationMatrix, rotationMatrix, scaleMatrix;
    std::string label;
    pmp::mat4 getModelMatrix()
    {
        return (scaleMatrix * rotationMatrix * translationMatrix);
    }
};

class Database
{
public:
    Database() = default;
    Database(const std::string path);
    void draw(const pmp::mat4& projection_matrix,
              const std::string& draw_mode);
    void retrieve(const std::string& path);
    pmp::mat4 getModelMatrx(int index)
    {
        return entries[index].getModelMatrix();
    }
    void clear();

private:
    int avgVerts, avgFaces;
    std::vector<Entry> entries;
};
} // namespace mmr
