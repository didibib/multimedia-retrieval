#pragma once

#include <pmp/visualization/MeshViewer.h>
#include "database.h"

namespace mmr {
class MmrViewer : public pmp::MeshViewer
{
public:
    MmrViewer(const char* title, int width, int height);

protected:
    void mouse(int button, int action, int mods) override;
    void keyboard(int key, int code, int action, int mod) override;
    void process_imgui() override;

    Database m_database;
};
} // namespace mmr
