#pragma once

#include <pmp/visualization/MeshViewer.h>
#include "database.h"

namespace mmr {
class MmrViewer : public pmp::MeshViewer
{
public:
    MmrViewer(const char* title, int width, int height);

protected:
    void draw(const std::string& draw_mode) override;
    void keyboard(int key, int code, int action, int mod) override;
    void process_imgui() override;

    Database m_database;

private:
    bool m_retrieved_db = false;
    int m_dbIndex = 0;
};
} // namespace mmr
