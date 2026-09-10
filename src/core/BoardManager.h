#pragma once

#include "core/Board.h"

#include <QList>
#include <vector>
#include <QObject>
#include <QString>
#include <QUuid>
#include <memory>

/**
 * BoardManager -- owns the full set of named Boards for the application.
 *
 * Design choices:
 *   - Inherits QObject for signals so the UI can react to board list changes.
 *   - Uses std::unique_ptr<Board> per board (Board is non-copyable QObject).
 *   - Ownership: MainWindow holds a std::unique_ptr<BoardManager>.
 *
 * Tier 3 Item 11: Multiple boards, switchable.
 */
struct BoardMeta {
    QUuid   id;
    QString name;
};

class BoardManager : public QObject
{
    Q_OBJECT

public:
    explicit BoardManager(QObject *parent = nullptr);

    // -- Queries -------------------------------------------------------
    int   count()                       const;
    int   activeIndex()                 const;
    Board *activeBoard()                const;
    Board *boardAt(int index)           const;
    BoardMeta metaAt(int index)         const;
    QList<BoardMeta> allMeta()          const;

    // -- Mutations -------------------------------------------------------
    // Add a new empty board; returns its index.
    int  addBoard(const QString &name);
    // Remove board at index (not allowed if only 1 board remains).
    bool removeBoard(int index);
    // Rename a board.
    void renameBoard(int index, const QString &name);
    // Switch the active board.
    void setActiveIndex(int index);

    // -- Direct board access for load/save --------------------------------
    // Used by JsonStore to set boards from serialized data.
    void setBoardsFromLoad(QList<BoardMeta> metas,
                           QList<QList<Task>> taskLists);

signals:
    void boardListChanged();      // emitted after add/remove/rename
    void activeBoardChanged(int index); // emitted after setActiveIndex

private:
    struct Entry {
        BoardMeta              meta;
        std::unique_ptr<Board> board;
    };

    std::vector<Entry> m_entries;
    int          m_activeIndex = 0;
};
