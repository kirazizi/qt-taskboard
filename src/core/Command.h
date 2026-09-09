#pragma once

#include "core/Task.h"

/**
 * Command -- abstract base class for the Command Pattern (Tier 2 Item 10).
 *
 * Design choices:
 *   - Pure C++ (no QObject) -- core/ must not include QWidget headers.
 *   - Each concrete command captures enough state to both execute() and undo().
 *   - Commands are executed once when pushed to CommandHistory, then undo/redo
 *     toggles them in place.
 *
 * Concrete commands:
 *   AddTaskCommand    -- add a task to the board
 *   RemoveTaskCommand -- remove a task from the board
 *   UpdateTaskCommand -- replace a task's fields (edit dialog result)
 *   MoveTaskCommand   -- change a task's column (drag & drop)
 */
class Board;

class Command
{
public:
    virtual ~Command() = default;

    // Execute (or re-execute after redo) the command
    virtual void execute(Board &board) = 0;

    // Revert the command's effect on the board
    virtual void undo(Board &board) = 0;

    // Human-readable description for debug/accessibility
    virtual const char *description() const = 0;
};

// -----------------------------------------------------------------------------
// AddTaskCommand -- records the Task so it can be re-added on redo
// -----------------------------------------------------------------------------
class AddTaskCommand : public Command
{
public:
    explicit AddTaskCommand(Task task);

    void execute(Board &board) override;
    void undo(Board &board) override;
    const char *description() const override { return "Add task"; }

private:
    Task m_task;
};

// -----------------------------------------------------------------------------
// RemoveTaskCommand -- captures the full Task so it can be restored on undo
// -----------------------------------------------------------------------------
class RemoveTaskCommand : public Command
{
public:
    explicit RemoveTaskCommand(Task task);

    void execute(Board &board) override;
    void undo(Board &board) override;
    const char *description() const override { return "Remove task"; }

private:
    Task m_task;
};

// -----------------------------------------------------------------------------
// UpdateTaskCommand -- stores before/after snapshots for bidirectional undo
// -----------------------------------------------------------------------------
class UpdateTaskCommand : public Command
{
public:
    UpdateTaskCommand(Task before, Task after);

    void execute(Board &board) override;
    void undo(Board &board) override;
    const char *description() const override { return "Update task"; }

private:
    Task m_before;
    Task m_after;
};

// -----------------------------------------------------------------------------
// MoveTaskCommand -- records old/new status for bidirectional undo
// -----------------------------------------------------------------------------
class MoveTaskCommand : public Command
{
public:
    MoveTaskCommand(QUuid id, Task::Status oldStatus, Task::Status newStatus);

    void execute(Board &board) override;
    void undo(Board &board) override;
    const char *description() const override { return "Move task"; }

private:
    QUuid        m_id;
    Task::Status m_oldStatus;
    Task::Status m_newStatus;
};
