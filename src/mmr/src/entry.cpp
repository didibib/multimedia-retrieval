#include "entry.h"
#include "util.h"

namespace mmr {

Entry::Entry(std::string filename, std::string label, std::string path,
             std::string db)
{
    mesh_path = path;
    std::filesystem::path p = filename;
    fv["filename"] = p.replace_extension().string(); // Remove extension
    fv["label"] = label;
    db_name = db;
    deserialize();
}

void Entry::updateStatistics()
{
    m_mesh.update_opengl_buffers();
    fv.updateStatistics(m_mesh);
}

void Entry::reload()
{
    if (!m_meshLoaded)
    {
        m_mesh.read(mesh_path);
        updateStatistics();
        m_meshLoaded = true;
    }
}

void Entry::draw(const pmp::mat4& projection_matrix,
                 const pmp::mat4& modelview_matrix,
                 const std::string& draw_mode)
{
    reload();
    m_mesh.draw(projection_matrix, modelview_matrix, draw_mode);
}

const void Entry::writeMesh(std::string extension, std::string folder)
{
    std::filesystem::path filename = Feature::toString(fv["filename"]);
    filename.replace_extension(extension);

    std::string label = Feature::toString(fv["label"]);
    std::string path = util::getExportDir(folder + "/" + label);

    m_mesh.write(path + "/" + filename.string());
}

void Entry::serialize()
{
    std::filesystem::path p = mesh_path;
    std::string filename = Feature::toString(fv["filename"]);
    std::string dir = p.parent_path().string() + "/" + filename;
    std::filesystem::create_directories(dir);
    fv.serialize(dir, filename);
}

void Entry::deserialize()
{
    std::filesystem::path p = mesh_path;
    std::string filename = Feature::toString(fv["filename"]);
    std::string folder = p.parent_path().string() + "/" + filename;
    if (!fv.deserialize(folder))
    {
        m_mesh.read(mesh_path);
        updateStatistics();
    }
}

} // namespace mmr