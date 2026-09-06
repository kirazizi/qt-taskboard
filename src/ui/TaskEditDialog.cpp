#include "ui/TaskEditDialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QTextEdit>
#include <QVBoxLayout>

static const QString kInputStyle = QStringLiteral(
    "background: #ffffff;"
    "border: 1px solid #e2e8f0;"
    "border-radius: 7px;"
    "padding: 8px 12px;"
    "color: #1a202c;"
    "font-size: 13px;"
);

static const QString kLabelStyle = QStringLiteral(
    "color: #718096; font-size: 12px; font-weight: 600;"
    " background: transparent; border: none;"
);

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
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(460, 400);
    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background: #ffffff;"
        "  border: 1px solid #cbd5e1;"
        "  border-radius: 12px;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // ── Header Row: Title on left, Close button on right ─────────────────
    auto *headerRow = new QWidget(this);
    headerRow->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    auto *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerLabel = new QLabel(windowTitle(), headerRow);
    headerLabel->setStyleSheet(QStringLiteral(
        "color: #1a202c; font-size: 17px; font-weight: 700;"
        " background: transparent; border: none;"
    ));

    auto *closeBtn = new QPushButton(QStringLiteral("✕"), headerRow);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: transparent; border: none;"
        "  color: #a0aec0; font-size: 13px; font-weight: 700;"
        "  border-radius: 6px;"
        "}"
        "QPushButton:hover { background: #edf2f7; color: #4a5568; }"
        "QPushButton:pressed { background: #e2e8f0; }"
    ));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(headerLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    mainLayout->addWidget(headerRow);

    // ── Form ─────────────────────────────────────────────────────────────
    auto *formLayout = new QGridLayout;
    formLayout->setVerticalSpacing(14);
    formLayout->setHorizontalSpacing(12);
    formLayout->setColumnMinimumWidth(0, 88);

    auto makeLabel = [&](const QString &text) {
        auto *l = new QLabel(text, this);
        l->setStyleSheet(kLabelStyle);
        return l;
    };

    // Title
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(QStringLiteral("Enter task title..."));
    m_titleEdit->setMaxLength(100);
    m_titleEdit->setStyleSheet(kInputStyle);
    formLayout->addWidget(makeLabel(QStringLiteral("Title")),       0, 0, Qt::AlignVCenter | Qt::AlignRight);
    formLayout->addWidget(m_titleEdit,                              0, 1);

    // Description
    m_descEdit = new QTextEdit(this);
    m_descEdit->setFixedHeight(80);
    m_descEdit->setPlaceholderText(QStringLiteral("Description (optional)"));
    m_descEdit->setStyleSheet(kInputStyle);
    formLayout->addWidget(makeLabel(QStringLiteral("Description")), 1, 0, Qt::AlignTop | Qt::AlignRight);
    formLayout->addWidget(m_descEdit,                               1, 1);

    // Priority
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem(QStringLiteral("Low"),    static_cast<int>(Task::Priority::Low));
    m_priorityCombo->addItem(QStringLiteral("Medium"), static_cast<int>(Task::Priority::Medium));
    m_priorityCombo->addItem(QStringLiteral("High"),   static_cast<int>(Task::Priority::High));
    m_priorityCombo->setCurrentIndex(1);
    m_priorityCombo->setStyleSheet(kInputStyle);
    formLayout->addWidget(makeLabel(QStringLiteral("Priority")),    2, 0, Qt::AlignVCenter | Qt::AlignRight);
    formLayout->addWidget(m_priorityCombo,                          2, 1);

    // Due date
    m_dueDateEdit = new QDateEdit(this);
    m_dueDateEdit->setDisplayFormat(QStringLiteral("MMM d, yyyy"));
    m_dueDateEdit->setDate(QDate::currentDate().addDays(7));
    m_dueDateEdit->setCalendarPopup(true);
    m_dueDateEdit->setStyleSheet(kInputStyle);
    formLayout->addWidget(makeLabel(QStringLiteral("Due date")),    3, 0, Qt::AlignVCenter | Qt::AlignRight);
    formLayout->addWidget(m_dueDateEdit,                            3, 1);

    mainLayout->addLayout(formLayout);

    // ── Divider ──────────────────────────────────────────────────────────
    auto *divider = new QWidget(this);
    divider->setFixedHeight(1);
    divider->setStyleSheet(QStringLiteral("background: #e2e8f0; border: none;"));
    mainLayout->addWidget(divider);

    // ── Buttons ──────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);

    auto *cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #f7fafc; border: 1px solid #e2e8f0;"
        "  border-radius: 7px; padding: 8px 22px;"
        "  color: #4a5568; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #edf2f7; }"
    ));

    auto *okBtn = new QPushButton(QStringLiteral("Save"), this);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setDefault(true);
    okBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #4299e1; border: none;"
        "  border-radius: 7px; padding: 8px 28px;"
        "  color: white; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #3182ce; }"
        "QPushButton:pressed { background: #2b6cb0; }"
    ));

    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    mainLayout->addLayout(btnRow);
}

void TaskEditDialog::populate(const Task &task)
{
    m_titleEdit->setText(task.title());
    m_descEdit->setPlainText(task.description());
    m_dueDateEdit->setDate(task.dueDate().isValid()
        ? task.dueDate()
        : QDate::currentDate().addDays(7));
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
    return Task(m_titleEdit->text().trimmed(),
                m_descEdit->toPlainText().trimmed(),
                priority,
                m_dueDateEdit->date(),
                Task::Status::ToDo);
}

void TaskEditDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void TaskEditDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void TaskEditDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}

void TaskEditDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            reject(); // Never stay minimized/hidden while modal
        }
    }
    QDialog::changeEvent(event);
}

void TaskEditDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}
