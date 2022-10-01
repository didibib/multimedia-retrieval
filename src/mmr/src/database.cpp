#include <filesystem>
#include <vector>
#include "database.h"

namespace mmr {
Database::Database(const std::string path) : avgVerts(0), avgFaces(0)
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
    int nModels = 0;
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
            Entry entry;
            entry.label = label;
            entry.mesh.read(fileName);
            entries.push_back(entry);
            std::cout << "Model: " << ++nModels << std::endl;
        }
    }
}

void Database::clear() {
    entries.clear();
}
} // namespace mmr