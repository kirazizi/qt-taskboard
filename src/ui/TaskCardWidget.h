#pragma once

#include "core/Task.h"
#include <QWidget>

class QLabel;

/**
 * TaskCardWidget -- visual representation of a single Task on the board.
 *
 * Drag & Drop: DRAG SOURCE. Starts a drag on mouse press and encodes the
 * task UUID in QMimeData so the BoardColumnWidget (drop target) can identify
 * which task was moved.
 */
class TaskCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskCardWidget(const Task &task, QWidget *parent = nullptr);

    QUuid taskId() const;
    void  updateFromTask(const Task &task);

signals:
    void editRequested(QUuid id);
    void deleteRequested(QUuid id);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void updatePriorityLabel(Task::Priority p);
    void updateDueDateLabel(QDate dueDate);

    Task     m_task;
    QLabel  *m_titleLabel    = nullptr;
    QLabel  *m_priorityLabel = nullptr;
    QLabel  *m_dueDateLabel  = nullptr;
    QWidget *m_actionBar     = nullptr;
    QPoint   m_dragStartPosition;
};
