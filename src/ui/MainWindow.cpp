#include "ui/MainWindow.h"
#include "core/Board.h"
#include "data/JsonStore.h"
#include "ui/BoardColumnWidget.h"
#include "ui/TaskEditDialog.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_board(std::make_unique<Board>())
{
    m_saveFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                 + QStringLiteral("/board.json");
    setupUi();
    connectSignals();
    JsonStore::load(*m_board, m_saveFile);
    onBoardReset();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("Qt Taskboard"));
    setMinimumSize(900, 600);
    resize(1100, 700);
    auto *toolbar = addToolBar(QStringLiteral("Main"));
    toolbar->setMovable(false);
    auto *addBtn = new QPushButton(QStringLiteral("+ Add Task"), toolbar);
    addBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #6366f1; color: white; border: none;"
        " padding: 6px 16px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background: #4f46e5; }"
    ));
    toolbar->addWidget(addBtn);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddTask);
    auto *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet(QStringLiteral("background: #f8fafc;"));
    auto *boardLayout = new QHBoxLayout(central);
    boardLayout->setContentsMargins(16, 16, 16, 16);
    boardLayout->setSpacing(16);
    m_columns[0] = new BoardColumnWidget(QStringLiteral("To Do"),      Task::Status::ToDo,       m_board.get(), central);
    m_columns[1] = new BoardColumnWidget(QStringLiteral("In Progress"), Task::Status::InProgress, m_board.get(), central);
    m_columns[2] = new BoardColumnWidget(QStringLiteral("Done"),       Task::Status::Done,        m_board.get(), central);
    for (auto *col : m_columns) {
        col->setMinimumWidth(260);
        boardLayout->addWidget(col, 1);
    }
}

void MainWindow::connectSignals()
{
    connect(m_board.get(), &Board::taskAdded,   this, &MainWindow::onTaskAdded);
    connect(m_board.get(), &Board::taskUpdated, this, &MainWindow::onTaskUpdated);
    connect(m_board.get(), &Board::taskRemoved, this, &MainWindow::onTaskRemoved);
    connect(m_board.get(), &Board::boardReset,  this, &MainWindow::onBoardReset);
    for (auto *col : m_columns) {
        connect(col, &BoardColumnWidget::editRequested,   this, &MainWindow::onEditRequested);
        connect(col, &BoardColumnWidget::deleteRequested, this, &MainWindow::onDeleteRequested);
    }
}

void MainWindow::onTaskAdded(const Task &task)
{
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    saveBoard();
}

void MainWindow::onTaskUpdated(const Task &task)
{
    for (auto *col : m_columns) col->removeCard(task.id());
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    saveBoard();
}

void MainWindow::onTaskRemoved(QUuid id)
{
    for (auto *col : m_columns) col->removeCard(id);
    saveBoard();
}

void MainWindow::onBoardReset()
{
    m_columns[0]->rebuildAll(m_board->tasksByStatus(Task::Status::ToDo));
    m_columns[1]->rebuildAll(m_board->tasksByStatus(Task::Status::InProgress));
    m_columns[2]->rebuildAll(m_board->tasksByStatus(Task::Status::Done));
}

void MainWindow::onAddTask()
{
    TaskEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Task t = dlg.task();
        if (!t.title().isEmpty())
            m_board->addTask(std::move(t));
    }
}

void MainWindow::onEditRequested(QUuid id)
{
    const auto opt = m_board->findTask(id);
    if (!opt.has_value()) return;
    TaskEditDialog dlg(opt.value(), this);
    if (dlg.exec() == QDialog::Accepted) {
        Task updated = dlg.task();
        updated.setStatus(opt.value().status());
        m_board->updateTask(updated);
    }
}

void MainWindow::onDeleteRequested(QUuid id)
{
    m_board->removeTask(id);
}

BoardColumnWidget *MainWindow::columnFor(Task::Status status) const
{
    const auto idx = static_cast<std::size_t>(static_cast<int>(status));
    return (idx < m_columns.size()) ? m_columns[idx] : nullptr;
}

void MainWindow::saveBoard()
{
    JsonStore::save(*m_board, m_saveFile);
}
