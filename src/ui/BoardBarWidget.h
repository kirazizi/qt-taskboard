#pragma once

#include <QWidget>
#include <QList>

class BoardManager;
class QPushButton;
class QHBoxLayout;

/**
 * BoardBarWidget -- horizontal strip showing all boards as clickable tabs.
 *
 * Clicking a tab switches the active board.
 * The "+" button opens a QInputDialog to create a new board.
 * The small "x" on each tab (when >1 board) deletes that board.
 *
 * Tier 3 Item 11.
 */
class BoardBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardBarWidget(BoardManager *manager, QWidget *parent = nullptr);

    // Call after BoardManager emits boardListChanged() to rebuild the strip.
    void refresh();

signals:
    void boardSwitched(int index);   // user clicked a board tab

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onAddBoard();

private:
    BoardManager *m_manager;
    QHBoxLayout  *m_layout;
    QList<QPushButton *> m_tabs;
};
