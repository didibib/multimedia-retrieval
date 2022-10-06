#pragma once

#include "database.h"

namespace mmr {
class DbGui
{
    static void guiStatistics(Database& db);
    static void guiHistogram(Database& db);
    static void guiAlgorithms(Database& db, const int& index);
    static void guiDataMenu(Database& db);
   
public:

    static void guiBeginMenu(Database& db);
};
} // namespace mmr