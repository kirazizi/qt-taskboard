#pragma once
#include "core/Task.h"
#include <QDialog>
#include <optional>
class QLineEdit;
class QTextEdit;
class QComboBox;
class QDateEdit;
/**
 * TaskEditDialog -- modal dialog to create or edit a Task.
 *
 * Usage (create):  TaskEditDialog dlg(this);
 * Usage (edit):    TaskEditDialog dlg(existingTask, this);
 * After exec() == QDialog::Accepted, call task() for result.
 *
 * QDialog runs its own mini event-loop inside exec() -- it
 * blocks until the user clicks OK or Cancel.
 */
class TaskEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TaskEditDialog(QWidget *parent = nullptr);
    explicit TaskEditDialog(const Task &task, QWidget *parent = nullptr);
    Task task() const;
private:
    void buildUi();
    void populate(const Task &task);
    QLineEdit  *m_titleEdit;
    QTextEdit  *m_descEdit;
    QComboBox  *m_priorityCombo;
    QDateEdit  *m_dueDateEdit;
    std::optional<QUuid> m_existingId;
};
