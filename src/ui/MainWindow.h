#pragma once

#include "core/BoardManager.h"
#include "core/CommandHistory.h"
#include "core/ThemeManager.h"
#include "ui/BoardColumnWidget.h"

#include <QMainWindow>
#include <QString>
#include <array>
#include <memory>
#include <optional>

class QLineEdit;
class QComboBox;
class QPushButton;
class BoardBarWidget;

/**
 * MainWindow -- top-level application window.
 *
 * Owns a BoardManager (all boards) and a per-board CommandHistory.
 * All task mutations go through CommandHistory so they are undoable.
 *
 * Tier 3 Item 11: BoardManager replaces the single Board.
 *   - switchBoard() tears down old Board connections and builds new ones.
 *   - BoardBarWidget sits between the toolbar and the column canvas.
 *
 * Tier 3 Item 14: ThemeManager applies the active QSS on startup and on toggle.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFilterChanged();
    void onAddTask();
    void onEditRequested(QUuid id);
    void onDeleteRequested(QUuid id);
    void onDetailsRequested(QUuid id);
    void onMoveRequested(QUuid id, Task::Status newStatus);

    void onTaskAdded(const Task &task);
    void onTaskUpdated(const Task &task);
    void onTaskRemoved(QUuid id);
    void onBoardReset();

    // Item 11: switch to a different board by index
    void switchBoard(int index);

private:
    void setupUi();
    void connectBoardSignals();  // (re-)connect m_board signals after switch
    void connectSignals();
    void saveAll();
    void rebuildTagFilterCombo();
    BoardColumnWidget *columnFor(Task::Status status) const;
    Board *activeBoard() const;

    // ── Core ─────────────────────────────────────────────────────────────────
    std::unique_ptr<BoardManager>    m_manager;   // Item 11: owns all boards
    std::unique_ptr<CommandHistory>  m_history;   // Item 10: per-board undo stack

    // ── UI ───────────────────────────────────────────────────────────────────
    std::array<BoardColumnWidget *, 3> m_columns{};
    BoardBarWidget *m_boardBar        = nullptr;  // Item 11
    QLineEdit      *m_searchEdit      = nullptr;
    QComboBox      *m_priorityFilter  = nullptr;
    QComboBox      *m_tagFilter       = nullptr;
    QPushButton    *m_undoBtn         = nullptr;
    QPushButton    *m_redoBtn         = nullptr;
    QPushButton    *m_themeToggleBtn  = nullptr;  // Item 14
    QPushButton    *m_statsBtn         = nullptr;  // Item 12

    QString m_saveFile;
};
