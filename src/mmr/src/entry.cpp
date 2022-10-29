#include "entry.h"
#include "util.h"
#include <pmp/algorithms/DifferentialGeometry.h>

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

const void Entry::write(std::string extension, std::string folder)
{
    std::filesystem::path filename = Feature::toString(features["filename"]);
    filename.replace_extension(extension);

    std::string label = Feature::toString(features["label"]);
    std::string path = util::getExportDir(folder + "/" + label);

    mesh.write(path + "/" + filename.string());
}

void Entry::serialize()
{
    std::filesystem::path p = original_path;
    std::string folder = p.parent_path().string();
    features.serialize(folder, Feature::toString(features["filename"]));
}

void Entry::deserialize()
{
    std::filesystem::path p = original_path;
    std::string folder = p.parent_path().string();
    features.deserialize(folder);
}

std::vector<std::string> Entry::getHeaders()
{
    static std::vector<std::string> headers = {
        "filename",       "label",      "n_vertices",
        "n_faces",        "face_type",  "distance_to_origin",
        "bb_distance",    "bb_volume",  "surface_area",
        "rectangularity", "area",       "volume",
        "compactness",    "sphericity", "eccentricity",
        "diameter"};
    return headers;
}

} // namespace mmr