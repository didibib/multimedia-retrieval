#pragma once

#include <pmp/visualization/MeshViewer.h>
#include "database.h"
#include "database_gui.h"

namespace mmr {
class MmrViewer : public pmp::MeshViewer
{
public:
    MmrViewer(const char* title, int width, int height);

protected:
    void draw(const std::string& draw_mode) override;
    void keyboard(int key, int code, int action, int mod) override;
    void process_imgui() override; 

    Database db;
    

private:
    DbGui m_dbGui;
    bool m_retrieved_db = false;
    int m_selectedEntry = -1;
};
} // namespace mmr
