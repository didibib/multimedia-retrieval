#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include "pmp/MatVec.h"
#include <set>

namespace mmr {
    struct Entry
    {
    public:
        pmp::SurfaceMeshGL mesh;
        std::string label;
        std::string fileName;
        int nVertices;
        int nFaces;
        int volume;
        int surfaceArea;
    };

    class Database
    {
    public:
        Database() = default;
        Database(const std::string path);
        void retrieve(const std::string& path);
        void draw(const pmp::mat4& projection_matrix,
                  const pmp::mat4& modelview_matrix,
                  const std::string& draw_mode);
        void drawModel(int index, const pmp::mat4& projection_matrix,
                  const pmp::mat4& modelview_matrix,
                  const std::string& draw_mode);
        void clear();

        int getDBSize() { return m_entries.size(); }
        int getAvgVerts() { return m_avgVerts; }
        int getAvgFaces() { return m_avgFaces; }
        int getLabelCount() { return m_avgFaces; }

        void exportStatistics(std::string suffix = "");
        void exportMeshes(std::string folder);

    private:   
        std::vector<Entry> m_entries;

        int m_avgVerts = 0;
        int m_avgFaces = 0;

    };
} // namespace mmr
