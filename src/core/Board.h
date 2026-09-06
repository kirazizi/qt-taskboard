#pragma once

#include "core/Task.h"

#include <QList>
#include <QObject>
#include <optional>

/**
 * Board -- owns the full collection of Tasks and emits signals when it changes.
 *
 * Design choices:
 *   - Inherits QObject so it can emit signals to the UI layer.
 *   - QObjects are non-copyable by design -- use pointers to pass around.
 *   - Ownership: MainWindow will hold a std::unique_ptr<Board>.
 *     We don't use Qt parent-child here because Board is a core/ object
 *     and should not be tied to a UI parent.
 */
class Board : public QObject
{
    Q_OBJECT

public:
    explicit Board(QObject *parent = nullptr);

    // ----------------------------------------------------------
    // Queries
    // ----------------------------------------------------------

    // All tasks (used by JsonStore to serialise everything)
    QList<Task> tasks() const;

    // Tasks filtered by column (used by UI to populate each column)
    QList<Task> tasksByStatus(Task::Status status) const;

    // Look up a single task by its UUID (returns nullopt if not found)
    std::optional<Task> findTask(QUuid id) const;

    // ----------------------------------------------------------
    // Mutations -- each one emits a signal so the UI can react
    // ----------------------------------------------------------

    void addTask(Task task);
    void removeTask(QUuid id);
    void updateTask(const Task &task);                // replaces by id
    void moveTask(QUuid id, Task::Status newStatus); // drag & drop action

    // Replace all tasks at once (used by JsonStore after loading)
    void setTasks(QList<Task> tasks);

signals:
    // UI connects to these to know when to repaint
    void taskAdded(const Task &task);
    void taskRemoved(QUuid id);
    void taskUpdated(const Task &task);
    void boardReset(); // emitted after setTasks() -- UI should full redraw

private:
    QList<Task> m_tasks;
};
