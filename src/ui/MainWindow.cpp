#include "ui/MainWindow.h"
#include "core/Board.h"
#include "data/JsonStore.h"
#include "ui/BoardColumnWidget.h"
#include "ui/TaskEditDialog.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>

namespace {
class BoardCanvasWidget : public QWidget
{
public:
    explicit BoardCanvasWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_OpaquePaintEvent, false);
        if (!m_sourcePixmap.load(QStringLiteral(":/background.png"))) {
            m_sourcePixmap.load(QStringLiteral("resources/images/background.png"));
        }
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        updateCachedPixmap();
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        if (!m_scaledPixmap.isNull()) {
            painter.drawPixmap(0, 0, m_scaledPixmap);
        } else if (!m_sourcePixmap.isNull()) {
            painter.drawPixmap(rect(), m_sourcePixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            QLinearGradient grad(0, 0, width(), height());
            grad.setColorAt(0, QColor(0xdd, 0xe6, 0xf0));
            grad.setColorAt(1, QColor(0xc8, 0xd8, 0xea));
            painter.fillRect(rect(), grad);
        }
    }

private:
    void updateCachedPixmap()
    {
        if (m_sourcePixmap.isNull() || width() <= 0 || height() <= 0) {
            m_scaledPixmap = QPixmap();
            return;
        }
        QSize targetSize = m_sourcePixmap.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
        QPixmap scaled = m_sourcePixmap.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // Align horizontally centered, vertically bottom-aligned so the mountain peaks stay visible
        int x = std::max(0, (scaled.width() - width()) / 2);
        int y = scaled.height() - height();
        if (y < 0) y = 0;
        if (x < 0) x = 0;

        m_scaledPixmap = scaled.copy(x, y, width(), height());
    }

    QPixmap m_sourcePixmap;
    QPixmap m_scaledPixmap;
};
} // namespace

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
    setWindowTitle(QStringLiteral("Taskboard"));
    setMinimumSize(900, 580);
    resize(1150, 720);

    // ── Toolbar ──────────────────────────────────────────────────────────
    auto *toolbar = addToolBar(QStringLiteral("Main"));
    toolbar->setMovable(false);

    // App name
    auto *appLabel = new QLabel(QStringLiteral("Taskboard"), toolbar);
    appLabel->setStyleSheet(QStringLiteral(
        "color: #1a202c; font-size: 15px; font-weight: 700;"
        " background: transparent; padding-left: 4px; padding-right: 12px;"
    ));
    toolbar->addWidget(appLabel);

    // Search bar (Item 6: Search text)
    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search tasks..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedWidth(220);
    m_searchEdit->setFixedHeight(32);
    toolbar->addWidget(m_searchEdit);

    // Spacing between search and priority
    auto *gap = new QWidget(toolbar);
    gap->setFixedWidth(8);
    gap->setStyleSheet(QStringLiteral("background: transparent;"));
    toolbar->addWidget(gap);

    // Priority filter dropdown (Item 6: Filter by priority)
    m_priorityFilter = new QComboBox(toolbar);
    m_priorityFilter->addItem(QStringLiteral("All Priorities"), -1);
    m_priorityFilter->addItem(QStringLiteral("Low"),            static_cast<int>(Task::Priority::Low));
    m_priorityFilter->addItem(QStringLiteral("Medium"),         static_cast<int>(Task::Priority::Medium));
    m_priorityFilter->addItem(QStringLiteral("High"),           static_cast<int>(Task::Priority::High));
    m_priorityFilter->setFixedWidth(135);
    m_priorityFilter->setFixedHeight(32);
    toolbar->addWidget(m_priorityFilter);

    // Flexible spacer pushing Add Task to the far right
    auto *spacer = new QWidget(toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("background: transparent;");
    toolbar->addWidget(spacer);

    // Add Task button – clean blue pill
    auto *addBtn = new QPushButton(QStringLiteral("+ Add Task"), toolbar);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(32);
    addBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #4299e1;"
        "  color: white;"
        "  border: none;"
        "  padding: 0 20px;"
        "  border-radius: 7px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background: #3182ce; }"
        "QPushButton:pressed { background: #2b6cb0; }"
    ));
    toolbar->addWidget(addBtn);

    auto *rpad = new QWidget(toolbar);
    rpad->setFixedWidth(4);
    rpad->setStyleSheet("background: transparent;");
    toolbar->addWidget(rpad);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddTask);

    // ── Board canvas with user background ────────────────────────────────
    auto *central = new BoardCanvasWidget(this);
    setCentralWidget(central);

    auto *boardLayout = new QHBoxLayout(central);
    boardLayout->setContentsMargins(20, 20, 20, 20);
    boardLayout->setSpacing(16);

    m_columns[0] = new BoardColumnWidget(QStringLiteral("To Do"),       Task::Status::ToDo,       m_board.get(), central);
    m_columns[1] = new BoardColumnWidget(QStringLiteral("In Progress"),  Task::Status::InProgress, m_board.get(), central);
    m_columns[2] = new BoardColumnWidget(QStringLiteral("Done"),         Task::Status::Done,        m_board.get(), central);

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

    if (m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    }
    if (m_priorityFilter) {
        connect(m_priorityFilter, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);
    }

    for (auto *col : m_columns) {
        connect(col, &BoardColumnWidget::editRequested,   this, &MainWindow::onEditRequested);
        connect(col, &BoardColumnWidget::deleteRequested, this, &MainWindow::onDeleteRequested);
    }
}

void MainWindow::onFilterChanged()
{
    const QString query = m_searchEdit ? m_searchEdit->text() : QString();
    std::optional<Task::Priority> priority;
    if (m_priorityFilter && m_priorityFilter->currentIndex() > 0) {
        priority = static_cast<Task::Priority>(m_priorityFilter->currentData().toInt());
    }
    for (auto *col : m_columns) {
        if (col) {
            col->setFilter(query, priority);
        }
    }
}

void MainWindow::onTaskAdded(const Task &task)
{
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    onFilterChanged();
    saveBoard();
}

void MainWindow::onTaskUpdated(const Task &task)
{
    for (auto *col : m_columns) col->removeCard(task.id());
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    onFilterChanged();
    saveBoard();
}

void MainWindow::onTaskRemoved(QUuid id)
{
    for (auto *col : m_columns) col->removeCard(id);
    onFilterChanged();
    saveBoard();
}

void MainWindow::onBoardReset()
{
    m_columns[0]->rebuildAll(m_board->tasksByStatus(Task::Status::ToDo));
    m_columns[1]->rebuildAll(m_board->tasksByStatus(Task::Status::InProgress));
    m_columns[2]->rebuildAll(m_board->tasksByStatus(Task::Status::Done));
    onFilterChanged();
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
