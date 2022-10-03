#include <imgui.h>
#include <pmp/algorithms/DifferentialGeometry.h>
#include "util.h"
#include "normalization.h"
#include "mmr_viewer.h"

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

    if (m_retrieved_db)
        m_database.drawModel(m_dbIndex, projection_matrix_, modelview_matrix_, drawMode);
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
        /*case GLFW_KEY_F:
        {
            
            break;
        }*/
        

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
    if (ImGui::Button("Next") && m_retrieved_db && (m_dbIndex < m_database.getDBSize() -1))
    {
        
        m_dbIndex++;
    }

    if (ImGui::Button("Previous") && m_retrieved_db &&
        (m_dbIndex > 0))
    {
        m_dbIndex--;
    }

    if (ImGui::Button("Flip"))
    {
        Norma::lize(mesh_);
        update_mesh();
    }
}
} // namespace mmr