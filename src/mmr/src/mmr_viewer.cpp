#include <imgui.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include "util.h"
#include "normalization.h"
#include "mmr_viewer.h"
#include "database_gui.h"
#include "entry.h"
#include "ANN/ANN.h"

using namespace pmp;
using namespace std;

namespace mmr {
MmrViewer::MmrViewer(const char* title, int width, int height)
    : MeshViewer(title, width, height), m_dbGui(db)
{
    set_draw_mode("Smooth Shading");
}

void MmrViewer::draw(const std::string& drawMode)
{
    static std::vector<Entry*> entries;
    static const float step = 1.1f;
    static int offset = 0;

    if (m_dbGui.newSelectedEntry())
    {
        entries.clear();
        float radius = 0;
        float n = 0;

        for (auto& i : m_dbGui.getSelectedEntries())
        {
            Entry* e = db.get(i);
            e->reload();
            entries.push_back(e);

            BoundingBox& bb = e->getMesh().bounds();
            radius += bb.size() * .5f;
            n += step;
        }
        set_scene(vec3(0, 0, 0), radius);
        offset = step * entries.size() * .5f;
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        auto mv = translation_matrix(vec3(step * i - offset, 0, 0)) *
             modelview_matrix_;
        entries[i]->draw(projection_matrix_, mv, drawMode);
    }
}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        // add your own keyboard action here
        default:
        {
            MeshViewer::keyboard(key, scancode, action, mods);
            break;
        }
    }
}

void MmrViewer::process_imgui()
{
    if (ImGui::BeginMainMenuBar())
    {
        m_dbGui.beginGui(db);
        ImGui::EndMainMenuBar();
    }
}
} // namespace mmr