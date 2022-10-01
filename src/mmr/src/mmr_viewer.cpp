#include "mmr_viewer.h"

#include <imgui.h>
#include <pmp/algorithms/DifferentialGeometry.h>

using namespace pmp;

namespace mmr {
MmrViewer::MmrViewer(const char* title, int width, int height)
    : MeshViewer(title, width, height)
{
    set_draw_mode("Smooth Shading");
    add_help_item("C", "Translate barycenter to origin");
}

void MmrViewer::mouse(int button, int action, int mods) {}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        case GLFW_KEY_C:
        {
            Point origin(0, 0, 0);
            Point center = centroid(mesh_);
            Point translation = origin - center;

            auto points = mesh_.get_vertex_property<Point>("v:point");

            for (auto v : mesh_.vertices())
            {
                auto vp = points[v];
                points[v] += translation;
            }

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
}
} // namespace mmr