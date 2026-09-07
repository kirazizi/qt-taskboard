#include "ui/BoardColumnWidget.h"
#include "core/Board.h"
#include "ui/TaskCardWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

BoardColumnWidget::BoardColumnWidget(const QString &title,
                                    Task::Status status,
                                    Board *board,
                                    QWidget *parent)
    : QWidget(parent)
    , m_status(status)
    , m_board(board)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_StyledBackground, true);

    // Frosted glass column card with rounded corners and soft border
    setObjectName(QStringLiteral("boardColumn"));
    setStyleSheet(QStringLiteral(
        "#boardColumn {"
        "  background: rgba(255, 255, 255, 0.85);"
        "  border: 1px solid rgba(210, 224, 238, 0.75);"
        "  border-radius: 12px;"
        "}"
    ));

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ── Header ────────────────────────────────────────────────────────────
    auto *header = new QWidget(this);
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setObjectName(QStringLiteral("colHeader"));
    header->setStyleSheet(QStringLiteral(
        "#colHeader {"
        "  background: transparent;"
        "  border-bottom: 1px solid rgba(226, 232, 240, 0.8);"
        "}"
    ));

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 12, 16, 12);
    headerLayout->setSpacing(8);

    auto *titleLabel = new QLabel(title, header);
    titleLabel->setStyleSheet(QStringLiteral(
        "color: #2d3748; font-weight: 700; font-size: 13px;"
        " background: transparent;"
    ));

    // Count badge – simple pill
    m_countLabel = new QLabel(QStringLiteral("0"), header);
    m_countLabel->setAlignment(Qt::AlignCenter);
    m_countLabel->setFixedHeight(20);
    m_countLabel->setMinimumWidth(24);
    m_countLabel->setStyleSheet(QStringLiteral(
        "background: #e2e8f0;"
        "color: #4a5568;"
        "border-radius: 10px;"
        "font-size: 11px;"
        "font-weight: 600;"
        "padding: 0 7px;"
    ));

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_countLabel);

    // ── Scrollable card list ──────────────────────────────────────────────
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
    ));

    auto *cardsContainer = new QWidget;
    cardsContainer->setStyleSheet(QStringLiteral("background: transparent;"));
    m_cardsLayout = new QVBoxLayout(cardsContainer);
    m_cardsLayout->setContentsMargins(10, 10, 10, 10);
    m_cardsLayout->setSpacing(8);
    m_cardsLayout->addStretch();

    scrollArea->setWidget(cardsContainer);

    outerLayout->addWidget(header);
    outerLayout->addWidget(scrollArea, 1);
}

void BoardColumnWidget::addCard(const Task &task)
{
    auto *card = new TaskCardWidget(task);
    connect(card, &TaskCardWidget::editRequested,
            this, &BoardColumnWidget::editRequested);
    connect(card, &TaskCardWidget::deleteRequested,
            this, &BoardColumnWidget::deleteRequested);
    card->setVisible(card->matches(m_filterQuery, m_filterPriority));
    m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);
    updateCount();
}

void BoardColumnWidget::removeCard(QUuid id)
{
    if (auto *card = findCard(id)) {
        m_cardsLayout->removeWidget(card);
        card->deleteLater();
        updateCount();
    }
}

void BoardColumnWidget::updateCard(const Task &task)
{
    if (auto *card = findCard(task.id())) {
        card->updateFromTask(task);
        card->setVisible(card->matches(m_filterQuery, m_filterPriority));
        updateCount();
    }
}

void BoardColumnWidget::setFilter(const QString &textQuery, std::optional<Task::Priority> priority)
{
    m_filterQuery = textQuery;
    m_filterPriority = priority;
    for (int i = 0; i < m_cardsLayout->count(); ++i) {
        if (auto *card = qobject_cast<TaskCardWidget *>(m_cardsLayout->itemAt(i)->widget())) {
            card->setVisible(card->matches(m_filterQuery, m_filterPriority));
        }
    }
    updateCount();
}

void BoardColumnWidget::rebuildAll(const QList<Task> &tasks)
{
    while (m_cardsLayout->count() > 1) {
        auto *item = m_cardsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    for (const Task &t : tasks)
        addCard(t);
    updateCount();
}

TaskCardWidget *BoardColumnWidget::findCard(QUuid id) const
{
    for (int i = 0; i < m_cardsLayout->count(); ++i) {
        auto *w = qobject_cast<TaskCardWidget *>(m_cardsLayout->itemAt(i)->widget());
        if (w && w->taskId() == id) return w;
    }
    return nullptr;
}

void BoardColumnWidget::updateCount()
{
    int visibleCount = 0;
    for (int i = 0; i < m_cardsLayout->count(); ++i) {
        auto *w = m_cardsLayout->itemAt(i)->widget();
        if (qobject_cast<TaskCardWidget *>(w) && w->isVisible()) {
            ++visibleCount;
        }
    }
    m_countLabel->setText(QString::number(visibleCount));
}

void BoardColumnWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
        event->acceptProposedAction();
}

void BoardColumnWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText())
        event->acceptProposedAction();
}

void BoardColumnWidget::dropEvent(QDropEvent *event)
{
    const QUuid id = QUuid::fromString(event->mimeData()->text());
    if (id.isNull()) return;
    m_board->moveTask(id, m_status);
    event->acceptProposedAction();
}
