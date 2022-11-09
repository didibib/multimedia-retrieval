#pragma once

#include "features.h"
#include <string>
#include <vector>
#include <filesystem>
#include <pmp/visualization/SurfaceMeshGL.h>

namespace mmr {

class FeatureVector;
class Entry
{
private:
    bool m_meshLoaded = false;
    pmp::SurfaceMeshGL m_mesh;

public:
    Entry(std::string filename, std::string label, std::string path,
          std::string db);


    void updateStatistics();

    void reload();
    void draw(const pmp::mat4& projection_matrix,
              const pmp::mat4& modelview_matrix,
              const std::string& draw_mode);

    const void writeMesh(std::string extension, std::string folder = "");

    void serialize();
    void deserialize();
    pmp::SurfaceMeshGL& getMesh()
    {
        if (m_mesh.n_vertices() == 0)
            m_mesh.read(mesh_path);
        return m_mesh;
    }   

public:
    FeatureVector fv;
    std::string mesh_path;
    std::string db_name;
    bool isNormalized = false;
};
} // namespace mmr