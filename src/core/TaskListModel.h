#pragma once

#include "core/Task.h"

#include <QAbstractListModel>
#include <QList>
#include <QUuid>

/**
 * TaskListModel -- QAbstractListModel holding tasks for a single board column.
 *
 * Design choices:
 *   - Lives in core/ because it is a data-layer adapter (no QWidget dependency).
 *   - Custom role TaskRole returns the full Task value for delegates.
 *   - All mutations (add/remove/update) use beginInsertRows/endInsertRows etc.
 *     to let Qt's model/view infrastructure handle view updates automatically.
 *
 * Tier 3 Item 13: QAbstractListModel / QSortFilterProxyModel refactor.
 */
class TaskListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // Custom data role to retrieve the full Task value from the model.
    static constexpr int TaskRole = Qt::UserRole + 1;

    explicit TaskListModel(QObject *parent = nullptr);

    // -- QAbstractListModel interface ----------------------------------------
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // -- Mutations -----------------------------------------------------------
    void setTasks(const QList<Task> &tasks);
    void addTask(const Task &task);
    void removeTask(QUuid id);
    void updateTask(const Task &task);

    // -- Queries -------------------------------------------------------------
    // Returns the Task at the given row index, or a default Task if out of range.
    Task taskAt(int row) const;

private:
    QList<Task> m_tasks;

    int rowForId(QUuid id) const;
};
