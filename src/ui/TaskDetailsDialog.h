#pragma once

#include "core/Task.h"
#include <QDialog>
#include <QPoint>

class QMouseEvent;
class QShowEvent;

/**
 * TaskDetailsDialog -- read-only dialog showing full task info + timestamps.
 *
 * Tier 2 Item 8:
 *   - Shows title, description, priority, due date, and tags.
 *   - Shows "Created at" and "Last modified at" timestamps.
 *   - Uses the same frameless style as TaskEditDialog for visual consistency.
 *   - Triggered via a new "Details" button on TaskCardWidget (or double-click).
 */
class TaskDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDetailsDialog(const Task &task, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void buildUi(const Task &task);

    QPoint m_dragPos;
    bool   m_dragging = false;
};
