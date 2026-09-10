#pragma once

#include "core/Command.h"

#include <QObject>
#include <memory>
#include <vector>

/**
 * CommandHistory -- manages undo/redo stacks for the Command Pattern (Item 10).
 *
 * Uses std::vector<std::unique_ptr<Command>> for move-only command ownership.
 */
class CommandHistory : public QObject
{
    Q_OBJECT

public:
    explicit CommandHistory(QObject *parent = nullptr);

    // Execute cmd and push it onto the undo stack. Clears redo stack.
    void push(std::unique_ptr<Command> cmd, Board &board);

    void undo(Board &board);
    void redo(Board &board);

    bool canUndo() const;
    bool canRedo() const;
    // Item 11: clear all history when switching boards
    void clear();

signals:
    void canUndoChanged(bool enabled);
    void canRedoChanged(bool enabled);

private:
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};
