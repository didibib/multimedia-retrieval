#pragma once

#include "database.h"

namespace mmr {
class DbGui
{
    void window(Database& db);
    void statisticsTable(Database& db);
    void algorithmsPopup(Database& db, const int& row, const int& column);
    void normalizationMenu(Entry& entry);
    int columnIndex(std::string key);

    bool m_showHistogram = false;
    bool m_showStatistics = false;
    int m_selectedEntry = 0;
    Database& m_db;

public:
    DbGui(Database& db) : m_db(db) {}
    int getSelectedEntry() { return m_selectedEntry; }
    void beginMenu(Database& db);
};
} // namespace mmr