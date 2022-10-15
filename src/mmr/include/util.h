#pragma once
#include <pmp/MatVec.h>

using namespace pmp;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace mmr {
#define TARGET_VALUE 5000

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

std::mt19937::result_type seed =
    std::chrono::high_resolution_clock::now().time_since_epoch().count();
auto randomTargetIdx = std::bind(std::uniform_int_distribution<int>(0, TARGET_VALUE - 1),
                        std::mt19937(seed));

} // namespace util
} // namespace mmr