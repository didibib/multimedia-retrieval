#include "entry.h"
#include "util.h"

namespace mmr {

const void Entry::write(std::string extension, std::string folder)
{
    std::filesystem::path filename = Feature::toString(features["filename"]);
    filename.replace_extension(extension);

    std::string label = Feature::toString(features["label"]);
    std::string path = util::getExportDir(folder + "/" + label);

    mesh.write(path + "/" + filename.string());
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