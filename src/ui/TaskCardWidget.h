#pragma once

#include "core/Task.h"

#include <QWidget>

class QLabel;

/**
 * TaskCardWidget -- visual representation of a single Task on the board.
 *
 * Ownership: Qt parent-child tree (parent = the column widget that holds it).
 * Drag & Drop: This widget is the DRAG SOURCE. It starts a drag on
 * mouse press and encodes the task UUID in QMimeData so the
 * BoardColumnWidget (drop target) knows which task was dropped.
 */
class TaskCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskCardWidget(const Task &task, QWidget *parent = nullptr);

    // Return the task this card represents (used by column when deleting)
    QUuid taskId() const;

    // Refresh displayed data after a task is edited
    void updateFromTask(const Task &task);

signals:
    // Emitted when the user clicks the edit button on this card
    void editRequested(QUuid id);
    // Emitted when the user clicks the delete button
    void deleteRequested(QUuid id);

protected:
    // Override mousePressEvent to start a drag operation
    void mousePressEvent(QMouseEvent *event) override;

private:
    void updatePriorityLabel(Task::Priority p);
    void updateDueDateLabel(QDate dueDate);

    Task    m_task;
    QLabel *m_titleLabel;
    QLabel *m_priorityLabel;
    QLabel *m_dueDateLabel;
};

