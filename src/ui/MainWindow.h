#pragma once

#include "core/Board.h"
#include "core/CommandHistory.h"
#include "ui/BoardColumnWidget.h"

#include <QMainWindow>
#include <QString>
#include <array>
#include <memory>
#include <optional>

class QLineEdit;
class QComboBox;
class QPushButton;

/**
 * MainWindow -- top-level application window.
 *
 * Owns the Board (core), CommandHistory (core), and three BoardColumnWidgets.
 * All task mutations go through CommandHistory so they are undoable.
 *
 * Tier 2 Item 10 design:
 *   - Board's raw addTask/removeTask/updateTask/moveTask are called ONLY via
 *     Command objects pushed to CommandHistory.
 *   - This keeps Board clean (no history coupling) and keeps command logic
 *     in core/ where it belongs.
 *   - Ctrl+Z / Ctrl+Y keyboard shortcuts + toolbar buttons wired to undo/redo.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;  // Item 9: save geometry

private slots:
    void onFilterChanged();
    void onAddTask();
    void onEditRequested(QUuid id);
    void onDeleteRequested(QUuid id);
    void onDetailsRequested(QUuid id);  // Item 8
    void onMoveRequested(QUuid id, Task::Status newStatus);  // Item 10

    void onTaskAdded(const Task &task);
    void onTaskUpdated(const Task &task);
    void onTaskRemoved(QUuid id);
    void onBoardReset();

private:
    void setupUi();
    void connectSignals();
    void saveBoard();
    void rebuildTagFilterCombo();
    BoardColumnWidget *columnFor(Task::Status status) const;

    std::unique_ptr<Board>             m_board;
    std::unique_ptr<CommandHistory>    m_history;   // Item 10

    std::array<BoardColumnWidget *, 3> m_columns{};
    QLineEdit  *m_searchEdit      = nullptr;
    QComboBox  *m_priorityFilter  = nullptr;
    QComboBox  *m_tagFilter       = nullptr;  // Item 7
    QPushButton *m_undoBtn        = nullptr;  // Item 10
    QPushButton *m_redoBtn        = nullptr;  // Item 10
    QString     m_saveFile;
};
