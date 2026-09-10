#pragma once

#include <QStyledItemDelegate>

/**
 * TaskCardDelegate -- custom QStyledItemDelegate that paints task cards
 * inside a QListView in BoardColumnWidget.
 *
 * Replaces TaskCardWidget (QPushButton-based cards). The delegate is a
 * flyweight: it is instantiated once per column view and paints every
 * visible row without creating a QWidget per card.
 *
 * Paint details (matching the existing TaskCardWidget visual style):
 *   - White rounded-corner card background with subtle shadow
 *   - Bold title text
 *   - Colored pastel priority badge (Low/Medium/High)
 *   - Colored pastel tag pills
 *   - Due date (right-aligned, muted)
 *
 * Interaction:
 *   - Single-click selects the card.
 *   - Double-click triggers detailsRequested via editorEvent.
 *   - Right-click context menu for Edit / Delete / Details (future).
 *
 * Tier 3 Item 13.
 */
class TaskCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskCardDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void detailsRequested(QUuid id);
    void editRequested(QUuid id);
    void deleteRequested(QUuid id);
};
