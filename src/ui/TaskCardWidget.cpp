#include "ui/TaskCardWidget.h"

#include <QApplication>
#include <QDrag>
#include <QEnterEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

// Deterministic pastel color palette for tag badges (Item 7)
// Index is derived from qHash(tagText) % palette size
static const char *kTagPalette[] = {
    "background:#ebf8ff;color:#3182ce;border:1px solid #bee3f8;",
    "background:#f0fff4;color:#276749;border:1px solid #9ae6b4;",
    "background:#faf5ff;color:#6b46c1;border:1px solid #d6bcfa;",
    "background:#fff5f5;color:#c53030;border:1px solid #feb2b2;",
    "background:#fffbeb;color:#b7791f;border:1px solid #fbd38d;",
    "background:#e6fffa;color:#234e52;border:1px solid #81e6d9;",
};
static constexpr int kTagPaletteSize = 6;

TaskCardWidget::TaskCardWidget(const Task &task, QWidget *parent)
    : QWidget(parent)
    , m_task(task)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName(QStringLiteral("taskCard"));
    setStyleSheet(QStringLiteral(
        "#taskCard {"
        "  background: #ffffff;"
        "  border: 1px solid #e2e8f0;"
        "  border-radius: 8px;"
        "}"
        "#taskCard:hover {"
        "  border: 1px solid #90cdf4;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(6);

    // ── Row 1: title + action buttons ────────────────────────────────────
    auto *topRow    = new QWidget(this);
    topRow->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(4);

    m_titleLabel = new QLabel(task.title(), topRow);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "color: #2d3748; font-weight: 600; font-size: 13px; background: transparent;"
    ));

    // Action bar: opacity=0 when not hovered, revealed on hover
    m_actionBar = new QWidget(topRow);
    m_actionBar->setStyleSheet(QStringLiteral("background: transparent;"));
    m_actionBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto *actionLayout = new QHBoxLayout(m_actionBar);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(2);

    // Details button (Item 8) -- shows info icon
    auto *detailsBtn = new QPushButton(QStringLiteral("\u2139"), m_actionBar);
    auto *editBtn    = new QPushButton(QStringLiteral("..."),    m_actionBar);
    auto *delBtn     = new QPushButton(QStringLiteral("x"),      m_actionBar);

    detailsBtn->setFixedSize(24, 24);
    editBtn->setFixedSize(24, 24);
    delBtn->setFixedSize(24, 24);
    detailsBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setCursor(Qt::PointingHandCursor);

    const QString btnBase = QStringLiteral(
        "QPushButton {"
        "  background: transparent; border: none;"
        "  color: #a0aec0; font-size: %1px; font-weight: 700;"
        "  border-radius: 4px;"
        "}"
    );
    detailsBtn->setStyleSheet(
        btnBase.arg(14) +
        QStringLiteral("QPushButton:hover { background: #ebf8ff; color: #4299e1; }"));
    editBtn->setStyleSheet(
        btnBase.arg(14) +
        QStringLiteral("QPushButton:hover { background: #ebf8ff; color: #4299e1; }"));
    delBtn->setStyleSheet(
        btnBase.arg(12) +
        QStringLiteral("QPushButton:hover { background: #fff5f5; color: #e53e3e; }"));

    actionLayout->addWidget(detailsBtn);
    actionLayout->addWidget(editBtn);
    actionLayout->addWidget(delBtn);

    auto *opacityEffect = new QGraphicsOpacityEffect(m_actionBar);
    opacityEffect->setOpacity(0.0);
    m_actionBar->setGraphicsEffect(opacityEffect);
    m_actionBar->setEnabled(false);

    topLayout->addWidget(m_titleLabel, 1);
    topLayout->addWidget(m_actionBar, 0, Qt::AlignTop);

    // ── Row 2: tag badges (Item 7) ────────────────────────────────────────
    m_tagsRow = new QWidget(this);
    m_tagsRow->setStyleSheet(QStringLiteral("background: transparent;"));
    m_tagsLayout = new QHBoxLayout(m_tagsRow);
    m_tagsLayout->setContentsMargins(0, 0, 0, 0);
    m_tagsLayout->setSpacing(4);
    m_tagsLayout->addStretch();
    m_tagsRow->setVisible(!task.tags().isEmpty());

    // ── Row 3: priority badge + due date ─────────────────────────────────
    auto *metaRow    = new QWidget(this);
    metaRow->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *metaLayout = new QHBoxLayout(metaRow);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(8);

    m_priorityLabel = new QLabel(this);
    m_dueDateLabel  = new QLabel(this);

    updatePriorityLabel(task.priority());
    updateDueDateLabel(task.dueDate());

    metaLayout->addWidget(m_priorityLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_dueDateLabel);

    mainLayout->addWidget(topRow);
    mainLayout->addWidget(m_tagsRow);
    mainLayout->addWidget(metaRow);

    // Build initial tag badges
    updateTagBadges();

    connect(detailsBtn, &QPushButton::clicked, this, [this]() {
        emit detailsRequested(m_task.id());
    });
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit editRequested(m_task.id());
    });
    connect(delBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_task.id());
    });
}

QUuid TaskCardWidget::taskId() const { return m_task.id(); }

bool TaskCardWidget::matches(const QString &query,
                             std::optional<Task::Priority> priority,
                             const QStringList &tagFilter) const
{
    if (priority.has_value() && m_task.priority() != *priority)
        return false;

    const QString trimmed = query.trimmed();
    if (!trimmed.isEmpty()) {
        const bool titleMatch = m_task.title().contains(trimmed, Qt::CaseInsensitive);
        const bool descMatch  = m_task.description().contains(trimmed, Qt::CaseInsensitive);
        if (!titleMatch && !descMatch)
            return false;
    }

    // Item 7: tag filter -- task must have at least one tag in the filter list
    if (!tagFilter.isEmpty()) {
        bool tagMatch = false;
        for (const QString &filterTag : tagFilter) {
            if (m_task.tags().contains(filterTag, Qt::CaseInsensitive)) {
                tagMatch = true;
                break;
            }
        }
        if (!tagMatch)
            return false;
    }

    return true;
}

void TaskCardWidget::updateFromTask(const Task &task)
{
    m_task = task;
    m_titleLabel->setText(task.title());
    updatePriorityLabel(task.priority());
    updateDueDateLabel(task.dueDate());
    updateTagBadges();  // Item 7: refresh pills
}

void TaskCardWidget::updateTagBadges()
{
    // Clear existing badges (keep the stretch spacer at the end)
    while (m_tagsLayout->count() > 1) {
        auto *item = m_tagsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const QStringList tags = m_task.tags();
    for (const QString &tag : tags) {
        auto *badge = new QLabel(tag, m_tagsRow);
        const int colorIdx = static_cast<int>(qHash(tag) % static_cast<uint>(kTagPaletteSize));
        badge->setStyleSheet(
            QString::fromLatin1(kTagPalette[colorIdx]) +
            QStringLiteral(" border-radius: 4px; font-size: 10px;"
                           " font-weight: 600; padding: 1px 6px;"));
        // Insert before the stretch
        m_tagsLayout->insertWidget(m_tagsLayout->count() - 1, badge);
    }

    m_tagsRow->setVisible(!tags.isEmpty());
}

void TaskCardWidget::updatePriorityLabel(Task::Priority p)
{
    switch (p) {
        case Task::Priority::Low:
            m_priorityLabel->setText(QStringLiteral("Low"));
            m_priorityLabel->setStyleSheet(QStringLiteral(
                "background: #f0fff4; color: #38a169;"
                " border-radius: 4px; font-size: 11px; font-weight: 600;"
                " padding: 2px 8px; border: 1px solid #c6f6d5;"
            ));
            break;
        case Task::Priority::Medium:
            m_priorityLabel->setText(QStringLiteral("Medium"));
            m_priorityLabel->setStyleSheet(QStringLiteral(
                "background: #fffbeb; color: #d97706;"
                " border-radius: 4px; font-size: 11px; font-weight: 600;"
                " padding: 2px 8px; border: 1px solid #fde68a;"
            ));
            break;
        case Task::Priority::High:
            m_priorityLabel->setText(QStringLiteral("High"));
            m_priorityLabel->setStyleSheet(QStringLiteral(
                "background: #fff5f5; color: #e53e3e;"
                " border-radius: 4px; font-size: 11px; font-weight: 600;"
                " padding: 2px 8px; border: 1px solid #fed7d7;"
            ));
            break;
    }
}

void TaskCardWidget::updateDueDateLabel(QDate dueDate)
{
    if (!dueDate.isValid()) {
        m_dueDateLabel->setText(QString());
        return;
    }
    m_dueDateLabel->setText(dueDate.toString(QStringLiteral("MMM d, yyyy")));
    m_dueDateLabel->setStyleSheet(QStringLiteral(
        "color: #a0aec0; font-size: 11px; background: transparent;"
    ));
}

void TaskCardWidget::enterEvent(QEnterEvent *event)
{
    if (auto *eff = qobject_cast<QGraphicsOpacityEffect *>(m_actionBar->graphicsEffect())) {
        eff->setOpacity(1.0);
    }
    m_actionBar->setEnabled(true);
    QWidget::enterEvent(event);
}

void TaskCardWidget::leaveEvent(QEvent *event)
{
    if (auto *eff = qobject_cast<QGraphicsOpacityEffect *>(m_actionBar->graphicsEffect())) {
        eff->setOpacity(0.0);
    }
    m_actionBar->setEnabled(false);
    QWidget::leaveEvent(event);
}

void TaskCardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void TaskCardWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    auto *mime = new QMimeData;
    mime->setText(m_task.id().toString(QUuid::WithoutBraces));

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);
    const QPixmap pm = grab();
    if (!pm.isNull()) {
        drag->setPixmap(pm.scaled(200, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        drag->setHotSpot(QPoint(100, 30));
    }
    drag->exec(Qt::MoveAction);
}

void TaskCardWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit detailsRequested(m_task.id());  // Item 8: double-click opens details
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}
