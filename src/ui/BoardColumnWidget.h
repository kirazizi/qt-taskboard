#pragma once

#include "core/Task.h"
#include <QList>
#include <QStringList>
#include <QWidget>
#include <optional>

class Board;
class TaskCardWidget;
class QLabel;
class QVBoxLayout;
class QScrollArea;

/**
 * BoardColumnWidget -- one of the 3 Kanban columns
 * (To Do / In Progress / Done).
 *
 * Ownership: Qt parent-child tree (parent = MainWindow).
 * Drop target: calls board->moveTask(id, status) on drop.
 *
 * Tier 2 additions:
 *   - setFilter() now also accepts a QStringList tagFilter (Item 7)
 *   - Bubbles detailsRequested signal up to MainWindow (Item 8)
 */
class BoardColumnWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardColumnWidget(const QString &title,
                               Task::Status status,
                               Board *board,
                               QWidget *parent = nullptr);

    void addCard(const Task &task);
    void removeCard(QUuid id);
    void updateCard(const Task &task);
    void rebuildAll(const QList<Task> &tasks);
    // Item 11: switch to a new board and immediately rebuild cards
    void setBoardAndRebuild(Board *board, const QList<Task> &tasks);
    // Item 11: accessor so MainWindow can disconnect old board signals
    Board *board() const;

    // Item 7: extended filter includes tag filter
    void setFilter(const QString &textQuery,
                   std::optional<Task::Priority> priority,
                   const QStringList &tagFilter = {});

signals:
    void editRequested(QUuid id);
    void deleteRequested(QUuid id);
    void detailsRequested(QUuid id);  // Item 8
    void moveRequested(QUuid id, Task::Status newStatus);  // Item 10

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Task::Status                  m_status;
    Board                        *m_board;
    QVBoxLayout                  *m_cardsLayout;
    QLabel                       *m_countLabel;
    QString                       m_filterQuery;
    std::optional<Task::Priority> m_filterPriority;
    QStringList                   m_filterTags;   // Item 7

    TaskCardWidget *findCard(QUuid id) const;
    void updateCount();
};
