#pragma once
#include <pmp/MatVec.h>

using namespace pmp;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace mmr {
namespace util {

inline std::string getAssetDir()
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string assetDir = TOSTRING(MMR_ASSETS_DIR);
    return assetDir + "/";
}

inline std::string getExportDir()
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string exportDir = TOSTRING(MMR_EXPORT_DIR);
    return exportDir + "/";
}
} // namespace util

namespace asset {

inline std::string getShader(std::string name)
{
    return util::getAssetDir() + "shaders/" + name;
}

inline std::string getModel(std::string name)
{
    return util::getAssetDir() + "models/" + name;
}

} // namespace asset
} // namespace mmr