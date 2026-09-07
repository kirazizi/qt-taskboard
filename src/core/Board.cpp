#include "core/Board.h"

Board::Board(QObject *parent)
    : QObject(parent)
{}

// ---------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------

QList<Task> Board::tasks() const
{
    return m_tasks;
}

QList<Task> Board::tasksByStatus(Task::Status status) const
{
    QList<Task> result;
    for (const Task &t : m_tasks) {
        if (t.status() == status)
            result.append(t);
    }
    return result;
}

QList<Task> Board::tasksFiltered(Task::Status status,
                                 const QString &textQuery,
                                 std::optional<Task::Priority> priority) const
{
    QList<Task> result;
    const QString query = textQuery.trimmed();
    for (const Task &t : m_tasks) {
        if (t.status() != status) {
            continue;
        }
        if (priority.has_value() && t.priority() != *priority) {
            continue;
        }
        if (!query.isEmpty()) {
            const bool titleMatch = t.title().contains(query, Qt::CaseInsensitive);
            const bool descMatch  = t.description().contains(query, Qt::CaseInsensitive);
            if (!titleMatch && !descMatch) {
                continue;
            }
        }
        result.append(t);
    }
    return result;
}

std::optional<Task> Board::findTask(QUuid id) const
{
    for (const Task &t : m_tasks) {
        if (t.id() == id)
            return t;
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------

void Board::addTask(Task task)
{
    m_tasks.append(task);
    emit taskAdded(m_tasks.last());
}

void Board::removeTask(QUuid id)
{
    const auto removed = m_tasks.removeIf([&id](const Task &t) {
        return t.id() == id;
    });
    if (removed > 0)
        emit taskRemoved(id);
}

void Board::updateTask(const Task &task)
{
    for (Task &t : m_tasks) {
        if (t.id() == task.id()) {
            t = task;
            emit taskUpdated(t);
            return;
        }
    }
}

void Board::moveTask(QUuid id, Task::Status newStatus)
{
    for (Task &t : m_tasks) {
        if (t.id() == id) {
            if (t.status() == newStatus) {
                return; // already in this status, preserve position
            }
            t.setStatus(newStatus);
            emit taskUpdated(t);
            return;
        }
    }
}

void Board::setTasks(QList<Task> tasks)
{
    m_tasks = std::move(tasks);
    emit boardReset();
}
