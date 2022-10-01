#include <imgui.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include "util.h"
#include "mmr_viewer.h"

using namespace pmp;

namespace mmr {
MmrViewer::MmrViewer(const char* title, int width, int height)
    : MeshViewer(title, width, height)
{
    set_draw_mode("Smooth Shading");
    add_help_item("C", "Translate barycenter to origin");
}

void MmrViewer::draw(const std::string& drawMode)
{
    MeshViewer::draw(drawMode);

    if (m_retrieved_db)
        m_database.draw(projection_matrix_, modelview_matrix_, drawMode);
}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        case GLFW_KEY_C:
        {
            centroid

            update_mesh();
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
    MeshViewer::process_imgui();

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Retrieve DB"))
    {
        m_database.retrieve(asset::getModel("LabeledDB_new"));
        m_retrieved_db = true;
    }
}
} // namespace mmr