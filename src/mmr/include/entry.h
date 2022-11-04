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
    std::string checkFaceType()
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
    bool m_meshLoaded = false;

public:
    Entry(std::string filename, std::string label, std::string path,
          std::string db);

    static std::vector<std::string> getHeaders();

    void updateStatistics();

    void reload();
    void draw(const pmp::mat4& projection_matrix,
              const pmp::mat4& modelview_matrix,
              const std::string& draw_mode);

    const void writeMesh(std::string extension, std::string folder = "");

    void serialize();
    void deserialize();

public:
    FeatureVector features;
    pmp::SurfaceMeshGL mesh;
    std::string original_path;
    std::string db_name;
    bool isNormalized = false;
};
} // namespace mmr