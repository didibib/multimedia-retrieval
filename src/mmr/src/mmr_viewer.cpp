#include "mmr_viewer.h"

#include <imgui.h>

using namespace pmp;

void MmrViewer::mouse(int button, int action, int mods) {}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        case GLFW_KEY_P:
        {
            for each (auto prop in mesh_.vertex_properties())
            {
                printf(prop.c_str());
            }
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

void MmrViewer::process_imgui() {
    MeshViewer::process_imgui();
    
}
