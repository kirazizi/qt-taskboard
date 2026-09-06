#pragma once
#include "core/Task.h"
#include <QMainWindow>
#include <memory>
#include <array>
class Board;
class BoardColumnWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private slots:
    void onAddTask();
    void onEditRequested(QUuid id);
    void onDeleteRequested(QUuid id);
    void onTaskAdded(const Task &task);
    void onTaskUpdated(const Task &task);
    void onTaskRemoved(QUuid id);
    void onBoardReset();
private:
    void setupUi();
    void connectSignals();
    void saveBoard();
    BoardColumnWidget *columnFor(Task::Status status) const;
    std::unique_ptr<Board> m_board;
    std::array<BoardColumnWidget *, 3> m_columns{};
    QString m_saveFile;
};
