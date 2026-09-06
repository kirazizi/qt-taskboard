#pragma once

#include "core/Task.h"
#include <QDialog>
#include <QPoint>
#include <optional>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QDateEdit;
class QMouseEvent;
class QShowEvent;

/**
 * TaskEditDialog -- modal dialog to create or edit a Task.
 *
 * Frameless modal design:
 *   - Completely eliminates OS minimize and maximize buttons.
 *   - Custom draggable header with title and close button.
 *   - Clean rounded card styling.
 *   - Safe minimize guard to prevent modal locking.
 */
class TaskEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskEditDialog(QWidget *parent = nullptr);
    explicit TaskEditDialog(const Task &task, QWidget *parent = nullptr);

    Task task() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void buildUi();
    void populate(const Task &task);

    QLineEdit  *m_titleEdit       = nullptr;
    QTextEdit  *m_descEdit        = nullptr;
    QComboBox  *m_priorityCombo   = nullptr;
    QDateEdit  *m_dueDateEdit     = nullptr;
    std::optional<QUuid> m_existingId;

    QPoint m_dragPos;
    bool   m_dragging = false;
};
