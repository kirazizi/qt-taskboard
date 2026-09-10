#include "core/TaskListModel.h"

TaskListModel::TaskListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

// ── QAbstractListModel interface ────────────────────────────────────────────

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0; // flat list, no hierarchy
    return m_tasks.size();
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size())
        return {};

    const Task &task = m_tasks[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return task.title();
    case TaskRole:
        // The delegate uses this to get the full Task value for painting.
        return QVariant::fromValue(task);
    default:
        return {};
    }
}

// ── Mutations ────────────────────────────────────────────────────────────────

void TaskListModel::setTasks(const QList<Task> &tasks)
{
    beginResetModel();
    m_tasks = tasks;
    endResetModel();
}

void TaskListModel::addTask(const Task &task)
{
    const int row = m_tasks.size();
    beginInsertRows(QModelIndex(), row, row);
    m_tasks.append(task);
    endInsertRows();
}

void TaskListModel::removeTask(QUuid id)
{
    const int row = rowForId(id);
    if (row < 0) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_tasks.removeAt(row);
    endRemoveRows();
}

void TaskListModel::updateTask(const Task &task)
{
    const int row = rowForId(task.id());
    if (row < 0) return;
    m_tasks[row] = task;
    const QModelIndex idx = index(row);
    emit dataChanged(idx, idx, {Qt::DisplayRole, TaskRole});
}

// ── Queries ─────────────────────────────────────────────────────────────────

Task TaskListModel::taskAt(int row) const
{
    if (row < 0 || row >= m_tasks.size())
        return Task();  // default-constructed, only returned on invalid index
    return m_tasks[row];
}

int TaskListModel::rowForId(QUuid id) const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id() == id) return i;
    }
    return -1;
}
