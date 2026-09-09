#pragma once

#include "core/Task.h"
#include <QWidget>
#include <optional>

class QLabel;
class QHBoxLayout;

/**
 * TaskCardWidget -- visual representation of a single Task on the board.
 *
 * Drag & Drop: DRAG SOURCE. Starts a drag on mouse press and encodes the
 * task UUID in QMimeData so the BoardColumnWidget (drop target) can identify
 * which task was moved.
 *
 * Tier 2 additions:
 *   - Tag pill badges area (Item 7)
 *   - matches() updated to include tag filter (Item 7)
 *   - detailsRequested signal + Details button in action bar (Item 8)
 */
class TaskCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskCardWidget(const Task &task, QWidget *parent = nullptr);

    QUuid taskId() const;
    const Task &task() const { return m_task; }

    // Returns true if this card matches the given text query, priority filter,
    // and tag filter (any match on tagFilter is sufficient). (Items 6 + 7)
    bool matches(const QString &query,
                 std::optional<Task::Priority> priority,
                 const QStringList &tagFilter = {}) const;

    void updateFromTask(const Task &task);

signals:
    void editRequested(QUuid id);
    void deleteRequested(QUuid id);
    void detailsRequested(QUuid id);  // Item 8: open details dialog

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void updatePriorityLabel(Task::Priority p);
    void updateDueDateLabel(QDate dueDate);
    void updateTagBadges();  // Item 7: rebuild tag pill row

    Task         m_task;
    QLabel      *m_titleLabel    = nullptr;
    QLabel      *m_priorityLabel = nullptr;
    QLabel      *m_dueDateLabel  = nullptr;
    QWidget     *m_actionBar     = nullptr;
    QWidget     *m_tagsRow       = nullptr;  // Item 7: pill container
    QHBoxLayout *m_tagsLayout    = nullptr;  // Item 7
    QPoint       m_dragStartPosition;
};
