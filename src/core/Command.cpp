#include "core/Command.h"
#include "core/Board.h"

// ── AddTaskCommand ────────────────────────────────────────────────────────────

AddTaskCommand::AddTaskCommand(Task task)
    : m_task(std::move(task))
{}

void AddTaskCommand::execute(Board &board)
{
    board.addTask(m_task);
}

void AddTaskCommand::undo(Board &board)
{
    board.removeTask(m_task.id());
}

// ── RemoveTaskCommand ─────────────────────────────────────────────────────────

RemoveTaskCommand::RemoveTaskCommand(Task task)
    : m_task(std::move(task))
{}

void RemoveTaskCommand::execute(Board &board)
{
    board.removeTask(m_task.id());
}

void RemoveTaskCommand::undo(Board &board)
{
    // Re-add the task with its original data preserved
    board.addTask(m_task);
}

// ── UpdateTaskCommand ─────────────────────────────────────────────────────────

UpdateTaskCommand::UpdateTaskCommand(Task before, Task after)
    : m_before(std::move(before))
    , m_after(std::move(after))
{}

void UpdateTaskCommand::execute(Board &board)
{
    board.updateTask(m_after);
}

void UpdateTaskCommand::undo(Board &board)
{
    board.updateTask(m_before);
}

// ── MoveTaskCommand ───────────────────────────────────────────────────────────

MoveTaskCommand::MoveTaskCommand(QUuid id, Task::Status oldStatus, Task::Status newStatus)
    : m_id(id)
    , m_oldStatus(oldStatus)
    , m_newStatus(newStatus)
{}

void MoveTaskCommand::execute(Board &board)
{
    board.moveTask(m_id, m_newStatus);
}

void MoveTaskCommand::undo(Board &board)
{
    board.moveTask(m_id, m_oldStatus);
}
