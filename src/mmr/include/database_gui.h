#pragma once

#include "database.h"

namespace mmr {
class DbGui
{
    static void window(Database& db);
    static void statistics(Database& db);
    static void algorithms(Database& db, const int& row, const int& column);
    static void histogram(Database& db);
    static bool m_showHistogram;

public:

    static void beginMenu(Database& db);
};
} // namespace mmr