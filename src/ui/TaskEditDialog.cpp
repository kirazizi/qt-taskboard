#include "ui/TaskEditDialog.h"
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

TaskEditDialog::TaskEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("New Task"));
    buildUi();
}

TaskEditDialog::TaskEditDialog(const Task &task, QWidget *parent)
    : QDialog(parent)
    , m_existingId(task.id())
{
    setWindowTitle(QStringLiteral("Edit Task"));
    buildUi();
    populate(task);
}

void TaskEditDialog::buildUi()
{
    setMinimumWidth(400);

    auto *form = new QFormLayout;

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(QStringLiteral("Task title..."));
    m_titleEdit->setMaxLength(100);
    form->addRow(QStringLiteral("Title"), m_titleEdit);

    m_descEdit = new QTextEdit(this);
    m_descEdit->setFixedHeight(80);
    m_descEdit->setPlaceholderText(QStringLiteral("Description (optional)..."));
    form->addRow(QStringLiteral("Description"), m_descEdit);

    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem(QStringLiteral("Low"),   static_cast<int>(Task::Priority::Low));
    m_priorityCombo->addItem(QStringLiteral("Medium"), static_cast<int>(Task::Priority::Medium));
    m_priorityCombo->addItem(QStringLiteral("High"),   static_cast<int>(Task::Priority::High));
    m_priorityCombo->setCurrentIndex(1);
    form->addRow(QStringLiteral("Priority"), m_priorityCombo);

    m_dueDateEdit = new QDateEdit(this);
    m_dueDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_dueDateEdit->setDate(QDate::currentDate().addDays(7));
    m_dueDateEdit->setCalendarPopup(true);
    form->addRow(QStringLiteral("Due date"), m_dueDateEdit);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(btns);
}

void TaskEditDialog::populate(const Task &task)
{
    m_titleEdit->setText(task.title());
    m_descEdit->setPlainText(task.description());
    m_dueDateEdit->setDate(task.dueDate().isValid()
        ? task.dueDate()
        : QDate::currentDate().addDays(7));
    // Find the combo index whose stored data matches the task priority
    const int p = static_cast<int>(task.priority());
    for (int i = 0; i < m_priorityCombo->count(); ++i) {
        if (m_priorityCombo->itemData(i).toInt() == p) {
            m_priorityCombo->setCurrentIndex(i);
            break;
        }
    }
}

Task TaskEditDialog::task() const
{
    const auto priority = static_cast<Task::Priority>(
        m_priorityCombo->currentData().toInt());

    if (m_existingId.has_value()) {
        return Task(m_existingId.value(),
                    m_titleEdit->text().trimmed(),
                    m_descEdit->toPlainText().trimmed(),
                    priority,
                    m_dueDateEdit->date(),
                    Task::Status::ToDo);
    }
    // Create mode: Task generates a fresh UUID internally
    return Task(m_titleEdit->text().trimmed(),
                m_descEdit->toPlainText().trimmed(),
                priority,
                m_dueDateEdit->date());
}
