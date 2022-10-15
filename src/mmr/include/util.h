#pragma once
#include <pmp/MatVec.h>
#include <filesystem>

using namespace pmp;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace mmr {
namespace util {

inline std::string getDataDir(std::string directories = "")
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string dataDir = TOSTRING(MMR_DATA_DIR);
    return dataDir + "/" + directories;
}

// Creates directories if not exists
inline std::string getExportDir(std::string directories = "")
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string exportDir = TOSTRING(MMR_EXPORT_DIR);
    std::string path = exportDir + "/" + directories;

    namespace fs = std::filesystem;
    fs::create_directories(path);
    return path;
}

} // namespace util
} // namespace mmr