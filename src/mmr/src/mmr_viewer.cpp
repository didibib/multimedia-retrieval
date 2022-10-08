#include <imgui.h>
#include <implot.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include "util.h"
#include "normalization.h"
#include "mmr_viewer.h"
#include "database_gui.h"

using namespace pmp;
using namespace std;

namespace mmr {
MmrViewer::MmrViewer(const char* title, int width, int height)
    : MeshViewer(title, width, height), m_dbGui(db)
{
    set_draw_mode("Smooth Shading");
    m_selectedEntry = m_dbGui.getSelectedEntry();
}

void MmrViewer::draw(const std::string& drawMode)
{    
    static Entry* entry = nullptr;
    if (entry == nullptr || m_selectedEntry != m_dbGui.getSelectedEntry())
    {
        entry = db.get(m_dbGui.getSelectedEntry());

        if (entry == nullptr)
            return;

        m_selectedEntry = m_dbGui.getSelectedEntry();
        BoundingBox& bb = entry->mesh.bounds();
        set_scene(bb.center(), bb.size() * .5f);
    }

    if (entry == nullptr)
        return;

    entry->mesh.draw(projection_matrix_, modelview_matrix_, drawMode);
}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        case GLFW_KEY_C:
        {
            //break;
        }
        break;

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
    /*if (ImGui::Button("Next") && m_retrieved_db &&
        (m_dbIndex < m_db.getDbSize() - 1))
    {
        m_dbIndex++;
    }

    if (ImGui::Button("Previous") && m_retrieved_db && (m_dbIndex > 0))
    {
        m_dbIndex--;
    }

    if (ImGui::Button("Flip"))
    {
        Normalization::all_steps(mesh_);
        update_mesh();
    }*/

    if (ImGui::BeginMainMenuBar())
    {
        m_dbGui.beginMenu(db);
        ImGui::EndMainMenuBar();
    }

    //ImPlot::ShowDemoWindow();
    //ImGui::ShowDemoWindow();
}
} // namespace mmr