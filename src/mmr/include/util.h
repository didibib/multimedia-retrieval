#pragma once
#include <pmp/MatVec.h>

using namespace pmp;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace mmr {
namespace util {

inline std::string getDataDir(std::string suffix = "")
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string dataDir = TOSTRING(MMR_DATA_DIR);
    return dataDir + "/" + suffix;
}

inline std::string getExportDir(std::string suffix = "")
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string exportDir = TOSTRING(MMR_EXPORT_DIR);
    return exportDir + "/" + suffix;
}

} // namespace util
} // namespace mmr