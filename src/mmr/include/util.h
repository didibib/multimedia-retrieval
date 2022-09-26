#include <pmp/MatVec.h>

using namespace pmp;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace mmr {
namespace util {



} // namespace util

namespace asset {
inline std::string getDirectory()
{
    // MMR_ASSETS_DIR is set during cmake building in root/CMakeLists.txt
    static std::string assetDir = TOSTRING(MMR_ASSETS_DIR);
    return assetDir + "/";
}

inline std::string getShader(std::string name)
{
    return getDirectory() + "shaders/" + name;
}

inline std::string getModel(std::string name)
{
    return getDirectory() + "models/" + name;
}
} // namespace asset
} // namespace mmr