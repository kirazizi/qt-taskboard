#include "core/BoardManager.h"

BoardManager::BoardManager(QObject *parent)
    : QObject(parent)
{
    // Start with one default board
    Entry e;
    e.meta = { QUuid::createUuid(), QStringLiteral("My Board") };
    e.board = std::make_unique<Board>();
    m_entries.push_back(std::move(e));
}

// ── Queries ────────────────────────────────────────────────────────────────

int BoardManager::count() const { return m_entries.size(); }

int BoardManager::activeIndex() const { return m_activeIndex; }

Board *BoardManager::activeBoard() const
{
    return boardAt(m_activeIndex);
}

Board *BoardManager::boardAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return nullptr;
    return m_entries[index].board.get();
}

BoardMeta BoardManager::metaAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return {};
    return m_entries[index].meta;
}

QList<BoardMeta> BoardManager::allMeta() const
{
    QList<BoardMeta> result;
    result.reserve(m_entries.size());
    for (const auto &e : m_entries)
        result.append(e.meta);
    return result;
}

// ── Mutations ──────────────────────────────────────────────────────────────

int BoardManager::addBoard(const QString &name)
{
    Entry e;
    e.meta = { QUuid::createUuid(), name };
    e.board = std::make_unique<Board>();
    m_entries.push_back(std::move(e));
    emit boardListChanged();
    return static_cast<int>(m_entries.size()) - 1;
}

bool BoardManager::removeBoard(int index)
{
    if (static_cast<int>(m_entries.size()) <= 1) return false; // must keep at least 1
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return false;

    m_entries.erase(m_entries.begin() + index);

    // Clamp active index
    if (m_activeIndex >= static_cast<int>(m_entries.size()))
        m_activeIndex = static_cast<int>(m_entries.size()) - 1;

    emit boardListChanged();
    emit activeBoardChanged(m_activeIndex);
    return true;
}

void BoardManager::renameBoard(int index, const QString &name)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return;
    m_entries[index].meta.name = name;
    emit boardListChanged();
}

void BoardManager::setActiveIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return;
    if (index == m_activeIndex) return;
    m_activeIndex = index;
    emit activeBoardChanged(index);
}

void BoardManager::setBoardsFromLoad(QList<BoardMeta> metas,
                                     QList<QList<Task>> taskLists)
{
    m_entries.clear();
    for (int i = 0; i < static_cast<int>(metas.size()); ++i) {
        Entry e;
        e.meta = metas[i];
        e.board = std::make_unique<Board>();
        if (i < static_cast<int>(taskLists.size()))
            e.board->setTasks(taskLists[i]);
        m_entries.push_back(std::move(e));
    }
    if (m_entries.empty()) {
        // Ensure at least one board always exists
        Entry e;
        e.meta = { QUuid::createUuid(), QStringLiteral("My Board") };
        e.board = std::make_unique<Board>();
        m_entries.push_back(std::move(e));
    }
    m_activeIndex = 0;
    emit boardListChanged();
    emit activeBoardChanged(0);
}
