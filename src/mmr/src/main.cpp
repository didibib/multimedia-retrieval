// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include "mmr_viewer.h"
#include "util.h"

using namespace mmr;

int main(int argc, char **argv)
{
    MmrViewer window("MMR", 800, 600);

    window.load_mesh(asset::getModel("bunny.obj").c_str());

    return window.run();
}