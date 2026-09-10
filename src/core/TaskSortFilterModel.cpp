#include "core/TaskSortFilterModel.h"
#include "core/TaskListModel.h"

TaskSortFilterModel::TaskSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

void TaskSortFilterModel::setTextQuery(const QString &query)
{
    m_textQuery = query;
    invalidateFilter();
}

void TaskSortFilterModel::setPriorityFilter(std::optional<Task::Priority> priority)
{
    m_priority = priority;
    invalidateFilter();
}

void TaskSortFilterModel::setTagFilter(const QStringList &tags)
{
    m_tags = tags;
    invalidateFilter();
}

bool TaskSortFilterModel::filterAcceptsRow(int sourceRow,
                                           const QModelIndex &sourceParent) const
{
    const QModelIndex idx   = sourceModel()->index(sourceRow, 0, sourceParent);
    const QVariant    var   = sourceModel()->data(idx, TaskListModel::TaskRole);
    if (!var.isValid()) return false;

    const Task task = var.value<Task>();

    // Text filter (title or description, case-insensitive)
    if (!m_textQuery.isEmpty()) {
        const bool titleMatch = task.title().contains(m_textQuery, Qt::CaseInsensitive);
        const bool descMatch  = task.description().contains(m_textQuery, Qt::CaseInsensitive);
        if (!titleMatch && !descMatch) return false;
    }

    // Priority filter
    if (m_priority.has_value() && task.priority() != m_priority.value())
        return false;

    // Tag filter (task must contain ALL requested tags)
    for (const QString &requiredTag : m_tags) {
        if (!task.tags().contains(requiredTag)) return false;
    }

    return true;
}
