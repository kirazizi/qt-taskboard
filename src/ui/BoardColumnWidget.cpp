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

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Header: title + task count badge
    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("colHeader"));
    header->setStyleSheet(QStringLiteral(
        "#colHeader { background: #f1f5f9; padding: 12px;"
        " border-bottom: 1px solid #e2e8f0; }"
    ));
    auto *headerLayout = new QHBoxLayout(header);

    auto *titleLabel = new QLabel(title, header);
    QFont f(titleLabel->font());
    f.setWeight(QFont::Bold);
    f.setPointSize(11);
    titleLabel->setFont(f);

    m_countLabel = new QLabel(QStringLiteral("0"), header);
    m_countLabel->setStyleSheet(QStringLiteral(
        "background: #e0e7ff; color: #4338ca; padding: 2px 8px;"
        " border-radius: 10px; font-size: 11px;"
    ));

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_countLabel);

    // Scrollable card list
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *cardsContainer = new QWidget;
    m_cardsLayout = new QVBoxLayout(cardsContainer);
    m_cardsLayout->setContentsMargins(8, 8, 8, 8);
    m_cardsLayout->setSpacing(6);
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
    if (auto *card = findCard(task.id()))
        card->updateFromTask(task);
}

void BoardColumnWidget::rebuildAll(const QList<Task> &tasks)
{
    while (m_cardsLayout->count() > 1)
    {
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
    // -1 because the last layout item is always the trailing stretch
    m_countLabel->setText(QString::number(m_cardsLayout->count() - 1));
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
