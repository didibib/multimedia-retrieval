#include <filesystem>
#include <vector>
#include "database.h"

namespace mmr {

int Database::avgFaces = 0;
int Database::avgVerts = 0;
Database::Database(const std::string path)
{
    retrieve(path);
}

void Database::draw(const pmp::mat4& projection_matrix,
                    const pmp::mat4& modelview_matrix,
                    const std::string& draw_mode)
{
    for (int i = 0; i < entries.size(); i++)
    {
        entries[i].mesh.draw(projection_matrix, modelview_matrix, draw_mode);
    }
}

void Database::retrieve(const std::string& path)
{
    using std::filesystem::recursive_directory_iterator;
    //std::string path = asset::getModel(name);
    int nModels = 1;
    int maxModels = 10;
    std::string label;
    for (const auto& file : recursive_directory_iterator(path))
    {
        std::string fileName = file.path().u8string();
        std::string extension = fileName.substr(fileName.size() - 4);

        if (extension.compare(".txt") && extension.compare(".off"))
        {
            int index;
            for (index = fileName.size(); index > 0; index--)
            {
                if (fileName[index - 1] == '\\')
                    break;
            }
            label = fileName.substr(index);
        }
        else if (extension.compare(".txt") == 0)
            continue;
        else if (extension.compare(".off") == 0)
        {
            if (nModels > maxModels)
                break;
            Entry entry;
            entry.label = label;
            entry.mesh.read(fileName);
            avgVerts += entry.mesh.vertices_size();
            avgFaces += entry.mesh.faces_size();
            entries.push_back(entry);
            std::cout << "Model: " << nModels++ << std::endl;
        }
    }
    avgVerts /= maxModels;
    avgFaces /= maxModels;
    std::cout << "Average Vertices: " << avgVerts << std::endl;
    std::cout << "Average Faces: " << avgFaces << std::endl;
}

void Database::clear() {
    entries.clear();
}
} // namespace mmr