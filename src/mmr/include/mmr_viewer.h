#pragma once

#include <pmp/visualization/MeshViewer.h>

class MmrViewer : public pmp::MeshViewer
{
public:
    //! constructor
    MmrViewer(const char* title, int width, int height)
        : MeshViewer(title, width, height)
    {
        set_draw_mode("Smooth Shading");
    }

protected:
    //! this function handles mouse button presses
    void mouse(int button, int action, int mods) override;

    //! this function handles keyboard events
    void keyboard(int key, int code, int action, int mod) override;

    //! draw the scene in different draw modes
    void process_imgui() override;
};
