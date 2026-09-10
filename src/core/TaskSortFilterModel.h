#pragma once

#include "core/Task.h"

#include <QSortFilterProxyModel>
#include <QStringList>
#include <optional>

/**
 * TaskSortFilterModel -- QSortFilterProxyModel that wraps a TaskListModel.
 *
 * Replaces the manual Board::tasksFiltered() loop with Qt's standard
 * model/view filtering infrastructure. The proxy sits between the model
 * and the view -- the view only sees the filtered subset.
 *
 * Filter rules (AND-combined):
 *   - textQuery: case-insensitive match against title or description.
 *   - priority:  exact priority match (nullopt = show all).
 *   - tagFilter: task must have ALL specified tags.
 *
 * Tier 3 Item 13.
 */
class TaskSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TaskSortFilterModel(QObject *parent = nullptr);

    void setTextQuery(const QString &query);
    void setPriorityFilter(std::optional<Task::Priority> priority);
    void setTagFilter(const QStringList &tags);

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    QString                       m_textQuery;
    std::optional<Task::Priority> m_priority;
    QStringList                   m_tags;
};
