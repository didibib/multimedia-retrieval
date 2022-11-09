#pragma once

#include "database.h"
#include <vector>
#include <imgui.h>

namespace mmr {
class DbGui
{
    void window(Database& db);
    void statisticsTable(Database& db);
    void rightClickEntry(Database& db, const int& row, const int& column);
    void normalizeEntry(Entry& entry);
    void normalizeAll(Database& db);
    int columnIndex(std::string key);
    void exportMenu(Database& db);
    void results(Database& db);
    void queryMenu(Database& db);
    void makeQuery(Database& db, int index, Database::NNmethod method);
    void KRmenu(Database& db);

    bool m_showHistogram = false;
    bool m_showStatistics = false;
    std::vector<int> m_selectedEntries;
    bool m_newSelection = false;
    void makeNewSelection()
    {
        m_selectedEntries.clear();
        m_newSelection = true;
    }

    Database& m_db;

    bool m_scoring = false;
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontDefault();

public:
    DbGui(Database& db) : m_db(db) {}
    std::vector<int> getSelectedEntries() { return m_selectedEntries; }
    void beginGui(Database& db);

    bool newSelection()
    {
        bool r = false;
        if (m_newSelection)
        {
            r = true;
            m_newSelection = false;
        }
        return r;
    }
};
} // namespace mmr