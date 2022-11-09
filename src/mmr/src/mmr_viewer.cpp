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
    pmp::IOFlags flags;
    flags.use_vertex_colors = true;
    flags.use_face_colors = true;

    axisx.read(util::getDataDir() + "axis_x.ply", flags);
    axisx.set_front_color(vec3(1, 0, 0));
    axisy.read(util::getDataDir() + "axis_y.ply", flags);
    axisy.set_front_color(vec3(0, 1, 0));
    axisz.read(util::getDataDir() + "axis_z.ply", flags);
    axisz.set_front_color(vec3(0, 0, 1));
    set_scene(vec3(0, 0, 0), 1);
}

void MmrViewer::draw(const std::string& drawMode)
{
    axisx.draw(projection_matrix_, modelview_matrix_, "Smooth Shading");
    axisy.draw(projection_matrix_, modelview_matrix_, "Smooth Shading");
    axisz.draw(projection_matrix_, modelview_matrix_, "Smooth Shading");

    static std::vector<Entry> entries;
    static const float step = 1.1f;
    static int offset = 0;

    if (m_dbGui.newSelectedEntry())
    {
        entries.clear();
        float radius = 0;
        float n = 0;

        for (auto& i : m_dbGui.getSelectedEntries())
        {
            Entry e = *db.get(i);
            entries.push_back(e);

            BoundingBox& bb = e.getMesh().bounds();
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
        entries[i].draw(projection_matrix_, mv, drawMode);
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

    //ImGui::ShowDemoWindow();
}
} // namespace mmr