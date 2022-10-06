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
    : MeshViewer(title, width, height)
{
    set_draw_mode("Smooth Shading");
}

void MmrViewer::draw(const std::string& drawMode)
{
    MeshViewer::draw(drawMode);
    
    Entry* entry = db.get(m_dbIndex);
    if (entry == nullptr)
        return;

    BoundingBox& bb = entry->mesh.bounds();
    set_scene(bb.center(), bb.size() * .5f);

    entry->mesh.draw(projection_matrix_, modelview_matrix_,
                                drawMode);
}

void MmrViewer::keyboard(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) // only react on key press events
        return;

    switch (key)
    {
        case GLFW_KEY_C:
        {
            auto bb = mesh_.bounds();
            cout << "Mesh bounds BEFORE " << bb.size() << endl;
            Norma::lize(mesh_);
            update_mesh();
            bb = mesh_.bounds();
            cout << "Mesh bounds AFTER " << bb.size() << endl;
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
        Norma::lize(mesh_);
        update_mesh();
    }*/

    if (ImGui::BeginMainMenuBar())
    {
        DbGui::beginMenu(db);
        ImGui::EndMainMenuBar();
    }

    //ImPlot::ShowDemoWindow();
    //ImGui::ShowDemoWindow();
}
} // namespace mmr