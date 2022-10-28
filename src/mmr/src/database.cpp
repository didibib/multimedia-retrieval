#include "database.h"
#include "descriptors.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "..\include\entry.h"

namespace mmr {

Entry::Entry(std::string filename, std::string label, std::string path,
             std::string db)
{
    original_path = path;
    mesh.read(path);
    std::filesystem::path p = filename;
    features["filename"] = p.replace_extension().string(); // Remove extension
    features["label"] = label;
    db_name = db;
    updateStatistics();
}

void Entry::updateStatistics()
{
    features["n_vertices"] = static_cast<int>(mesh.n_vertices());
    features["n_faces"] = static_cast<int>(mesh.n_faces());
    features["face_type"] = checkFaceType();
    features["distance_to_origin"] =
        pmp::distance(pmp::centroid(mesh), pmp::vec3(0, 0, 0));

    pmp::BoundingBox bb = mesh.bounds();

    features["bb_distance"] = pmp::distance(bb.max(), bb.min());
    features["surface_area"] = pmp::surface_area(mesh);

    features["bb_volume"] =
        ((bb.max()[0] - bb.min()[0]) * (bb.max()[1] - bb.min()[1]) *
         (bb.max()[2] - bb.min()[2]));
    features["rectangularity"] = (volume(mesh) / ((bb.max()[0] - bb.min()[0]) *
                                                  (bb.max()[1] - bb.min()[1]) *
                                                  (bb.max()[2] - bb.min()[2])));
    features["area"] = surface_area(mesh);
    features["volume"] = volume(mesh);
    Scalar compactness = Descriptor::compactness(mesh);
    features["compactness"] = compactness;
    features["sphericity"] = (1 / compactness);
    features["eccentricity"] = Descriptor::eccentricity(mesh);
    features["diameter"] = Descriptor::diameter(mesh);
    features.updateFeatureVector();
}

// DATABASE ==================================================================================
// ===========================================================================================

Database::Database(const std::string path)
{
    import(path);
}

Entry* Database::get(int index)
{
    if (index < 0 || index >= m_entries.size())
        return nullptr;

    return &m_entries[index];
}

void Database::import(const std::string& path_)
{
    using std::filesystem::recursive_directory_iterator;
    int nModels = 0;
    int maxModels = 380;
    int nQueries = 1;
    std::filesystem::path p = path_;
    name = p.filename().string();

    for (const auto& file_entry : recursive_directory_iterator(path_))
    {
        std::string path = file_entry.path().string();
        std::string filename = file_entry.path().filename().string();
        std::string extension = file_entry.path().extension().string();
        std::string parentPath = file_entry.path().parent_path().string();
        std::string label =
            parentPath.substr(parentPath.find_last_of("/\\") + 1);

        if (extension != ".off" && extension != ".ply")
            continue;

        if (nModels > maxModels)
            break;

        /*if (filename != "360.off")
            continue;*/

        // Create entry
        Entry entry(filename, label, path, name);
        if (nModels++ > 0)
        {
            // Update global statistics
            m_avgVerts += entry.mesh.n_vertices();
            m_avgFaces += entry.mesh.n_faces();

            m_labels.insert(label);
            m_entries.push_back(std::move(entry));
            std::cout << "Model: " << nModels << std::endl;
        }
        else
        {
            m_queries.push_back(std::move(entry));
        }
    }

    if ((nModels - nQueries) == 0 || nModels == 0)
        return;

    m_avgVerts /= (nModels - nQueries);
    m_avgFaces /= (nModels - nQueries);
    std::cout << "Average Vertices: " << m_avgVerts << std::endl;
    std::cout << "Average Faces: " << m_avgFaces << std::endl;

    m_imported = true;
    m_columns = m_entries[0].features.n_statistics();
}

void Database::clear()
{
    m_entries.clear();
}

void Database::exportStatistics(std::string suffix) const
{
    if (m_entries.size() == 0)
        return;

    std::string filename = "statistics";
    if (!suffix.empty())
        filename += "_" + suffix;

    std::ofstream statistics;
    statistics.open(util::getExportDir() + filename + ".csv");

    // Headers
    for (auto const& [key, val] : m_entries[0].features.statistics())
        statistics << key << ",";
    statistics << "\n";

    // Rows
    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        // Columns
        for (auto const& [key, val] : m_entries[i].features.statistics())
            statistics << Feature::toString(val) << ",";
        statistics << "\n";
    }
    statistics.close();
}

void Database::exportMeshes(std::string extension, std::string folder)
{
    for (auto& e : m_entries)
        e.write(extension, folder);
}
} // namespace mmr