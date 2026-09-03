#include "ui/TaskCardWidget.h"

#include <QDrag>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

TaskCardWidget::TaskCardWidget(const Task &task, QWidget *parent)
    : QWidget(parent)
    , m_task(task)
{
    setObjectName(QStringLiteral("taskCard"));
    setStyleSheet(QStringLiteral(
        "#taskCard { background: #fff; border: 1px solid #d1d5db;"
        " border-radius: 6px; padding: 8px; }"
        "#taskCard:hover { border-color: #6366f1; }"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    auto *topRow = new QWidget(this);
    auto *topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel(task.title(), topRow);
    m_titleLabel->setWordWrap(true);
    QFont f(this->font());
    f.setWeight(QFont::Bold);
    m_titleLabel->setFont(f);

    auto *editBtn = new QPushButton(QStringLiteral("✏"), topRow);
    auto *delBtn  = new QPushButton(QStringLiteral("✕"), topRow);
    editBtn->setFixedSize(24, 24);
    delBtn->setFixedSize(24, 24);
    editBtn->setStyleSheet(QStringLiteral("background:none; border:none;"));
    delBtn->setStyleSheet(QStringLiteral("background:none; border:none; color:#ef4444;"));

    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(editBtn);
    topLayout->addWidget(delBtn);

    // Priority and due date labels
    m_priorityLabel = new QLabel(this);
    m_dueDateLabel  = new QLabel(this);
    updatePriorityLabel(task.priority());
    updateDueDateLabel(task.dueDate());

    mainLayout->addWidget(topRow);
    mainLayout->addWidget(m_priorityLabel);
    mainLayout->addWidget(m_dueDateLabel);

    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit editRequested(m_task.id());
    });
    connect(delBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_task.id());
    });
}

QUuid TaskCardWidget::taskId() const { return m_task.id(); }

void TaskCardWidget::updateFromTask(const Task &task)
{
    m_task = task;
    m_titleLabel->setText(task.title());
    updatePriorityLabel(task.priority());
    updateDueDateLabel(task.dueDate());
}

void TaskCardWidget::updatePriorityLabel(Task::Priority p)
{
    switch (p) {
        case Task::Priority::Low:
            m_priorityLabel->setText(QStringLiteral("➣ Low"));
            m_priorityLabel->setStyleSheet(QStringLiteral("color: #22c55e; font-size: 11px;"));
            break;
        case Task::Priority::Medium:
            m_priorityLabel->setText(QStringLiteral("➣ Medium"));
            m_priorityLabel->setStyleSheet(QStringLiteral("color: #f59b0b; font-size: 11px;"));
            break;
        case Task::Priority::High:
            m_priorityLabel->setText(QStringLiteral("➣ High"));
            m_priorityLabel->setStyleSheet(QStringLiteral("color: #ef4444; font-size: 11px;"));
            break;
    }
}

void TaskCardWidget::updateDueDateLabel(QDate dueDate)
{
    if (!dueDate.isValid()) { m_dueDateLabel->setText(QString()); return; }
    m_dueDateLabel->setText(QStringLiteral("Ὄ1 ") + dueDate.toString(QStringLiteral("MMMd, yyyy")));
    m_dueDateLabel->setStyleSheet(QStringLiteral("color: #6b7280; font-size: 11px;"));
}

void TaskCardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    auto *mime = new QMimeData;
    mime->setText(m_task.id().toString(QUuid::WithoutBraces));

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);
    // grab() takes a screenshot of this widget to use as the dragging cursor image
    drag->setPixmap(grab().scaled(200, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drag->exec(Qt::MoveAction);
}
