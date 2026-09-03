#pragma once
#include "core/Task.h"
#include <QList>
#include <QWidget>
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

signals:
    // Bubbled up from TaskCardWidget to MainWindow
    void editRequested(QUuid id);
    void deleteRequested(QUuid id);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Task::Status      m_status;
    Board            *m_board;
    QVBoxLayout      *m_cardsLayout;
    QLabel           *m_countLabel;

    TaskCardWidget *findCard(QUuid id) const;
    void updateCount();
};
