#include "mmr_viewer.h"
#include "util.h"
#include "descriptors.h"

using namespace mmr;

int main(int argc, char **argv)
{
    std::vector<float> test = {0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3,
                               3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4,
                               4, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
                               7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9};

    auto h = Histogram("test", test, 0, 9, 10);
    h.save("test.txt");

    MmrViewer window("MMR", 800, 600);

    return window.run();
}