#include <imgui.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include "util.h"
#include "normalization.h"
#include "mmr_viewer.h"
#include "database_gui.h"
#include "entry.h"

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

    if (m_dbGui.newSelectedEntry())
    {
        entries.clear();
        float radius = 0;
        float n = 0;

        for (auto& i : m_dbGui.getSelectedEntries())
        {
            if (i == -1)
                continue;
            Entry* e = db.get(i);
            e->reload();
            entries.push_back(e);

            BoundingBox& bb = e->mesh.bounds();

            radius += bb.size() * .5f;
            n += step;
        }
        set_scene(vec3(n / 2 + step / 2, 0, 0), radius);
    }

    float popback = 0;
    for (size_t i = 0; i < entries.size(); i++)
    {
        translate(vec3(step, 0, 0));
        popback += step;
        entries[i]->draw(projection_matrix_, modelview_matrix_, drawMode);
    }
    translate(vec3(-popback, 0, 0));
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