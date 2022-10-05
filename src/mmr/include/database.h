#pragma once

#include "pmp/visualization/SurfaceMeshGL.h"
#include "pmp/MatVec.h"
#include <map>
#include <any>

namespace mmr {
    struct Entry
    {
    public:
        pmp::SurfaceMeshGL mesh;
        std::map<std::string, std::any> statistics;

    };

    class Database
    {
    public:
        Database() = default;
        Database(const std::string path);
        void retrieve(const std::string& path);
        void beginMenu();
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

    private:   
        std::vector<Entry> m_entries;

        int m_avgVerts = 0;
        int m_avgFaces = 0;

        void exportStatistics(std::string suffix = "");
        void exportMeshes(std::string folder);
        void guiDataMenu();
        void guiStatistics();
        bool m_imported = false;
        bool m_viewStatistics = false;

    };
} // namespace mmr
