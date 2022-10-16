#include "mmr_viewer.h"
#include "util.h"
#include "descriptors.h"

using namespace mmr;

int main(int argc, char **argv)
{
    MmrViewer window("MMR", 800, 600);

    return window.run();
}