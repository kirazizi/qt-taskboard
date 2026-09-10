#include "core/CommandHistory.h"
#include "core/Board.h"

CommandHistory::CommandHistory(QObject *parent)
    : QObject(parent)
{}

void CommandHistory::push(std::unique_ptr<Command> cmd, Board &board)
{
    // Execute the command before pushing it
    cmd->execute(board);

    // Any new action invalidates the redo stack (standard UX)
    const bool hadRedo = !m_redoStack.empty();
    m_redoStack.clear();

    const bool hadUndo = !m_undoStack.empty();
    m_undoStack.push_back(std::move(cmd));

    if (!hadUndo)
        emit canUndoChanged(true);
    if (hadRedo)
        emit canRedoChanged(false);
}

void CommandHistory::undo(Board &board)
{
    if (m_undoStack.empty()) return;

    const bool hadRedo = !m_redoStack.empty();

    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    cmd->undo(board);
    m_redoStack.push_back(std::move(cmd));

    emit canUndoChanged(!m_undoStack.empty());
    if (!hadRedo)
        emit canRedoChanged(true);
}

void CommandHistory::redo(Board &board)
{
    if (m_redoStack.empty()) return;

    const bool hadUndo = !m_undoStack.empty();

    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    cmd->execute(board);
    m_undoStack.push_back(std::move(cmd));

    emit canRedoChanged(!m_redoStack.empty());
    if (!hadUndo)
        emit canUndoChanged(true);
}

bool CommandHistory::canUndo() const { return !m_undoStack.empty(); }
bool CommandHistory::canRedo() const { return !m_redoStack.empty(); }

// Item 11: clear all history (called when switching boards)
void CommandHistory::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
    emit canUndoChanged(false);
    emit canRedoChanged(false);
}
