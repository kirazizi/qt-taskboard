#include "ui/MainWindow.h"

#include "core/Board.h"
#include "core/Command.h"
#include "core/CommandHistory.h"
#include "core/Task.h"
#include "data/JsonStore.h"
#include "ui/BoardColumnWidget.h"
#include "ui/TaskDetailsDialog.h"
#include "ui/TaskEditDialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSet>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QToolBar>
#include <QVBoxLayout>

// ── Background canvas widget (inline helper) ──────────────────────────────────
namespace {
class BoardCanvasWidget : public QWidget
{
public:
    explicit BoardCanvasWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_sourcePixmap.load(QStringLiteral(":/background.png"));
        if (m_sourcePixmap.isNull())
            m_sourcePixmap.load(QStringLiteral(":/images/background.png"));
        if (m_sourcePixmap.isNull())
            m_sourcePixmap.load(QStringLiteral("resources/images/background.png"));
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        updateCachedPixmap();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        if (!m_scaledPixmap.isNull()) {
            painter.drawPixmap(0, 0, m_scaledPixmap);
        } else {
            QLinearGradient grad(0, 0, 0, height());
            grad.setColorAt(0, QColor(0xe8, 0xf0, 0xf8));
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

// ── MainWindow ────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_board(std::make_unique<Board>())
    , m_history(std::make_unique<CommandHistory>())  // Item 10
{
    m_saveFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                 + QStringLiteral("/board.json");
    setupUi();
    connectSignals();
    JsonStore::load(*m_board, m_saveFile);

    // Item 9: restore window geometry from last session
    QSettings settings(QStringLiteral("qt-taskboard"), QStringLiteral("qt-taskboard"));
    const QByteArray geo = settings.value(QStringLiteral("window/geometry")).toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);
}

MainWindow::~MainWindow() = default;

// Item 9: save geometry + filter state
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings(QStringLiteral("qt-taskboard"), QStringLiteral("qt-taskboard"));
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    if (m_searchEdit)
        settings.setValue(QStringLiteral("filter/text"), m_searchEdit->text());
    if (m_priorityFilter)
        settings.setValue(QStringLiteral("filter/priority"), m_priorityFilter->currentIndex());
    if (m_tagFilter)
        settings.setValue(QStringLiteral("filter/tag"), m_tagFilter->currentData().toString());
    QMainWindow::closeEvent(event);
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("Taskboard"));
    setMinimumSize(900, 580);
    resize(1200, 720);

    // ── Toolbar ──────────────────────────────────────────────────────────
    auto *toolbar = addToolBar(QStringLiteral("Main"));
    toolbar->setMovable(false);

    auto *appLabel = new QLabel(QStringLiteral("Taskboard"), toolbar);
    appLabel->setStyleSheet(QStringLiteral(
        "color: #1a202c; font-size: 15px; font-weight: 700;"
        " background: transparent; padding-left: 4px; padding-right: 12px;"
    ));
    toolbar->addWidget(appLabel);

    auto makeGap = [&](int w) -> QWidget * {
        auto *g = new QWidget(toolbar);
        g->setFixedWidth(w);
        g->setStyleSheet(QStringLiteral("background: transparent;"));
        return g;
    };

    // Search bar
    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search tasks..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedWidth(200);
    m_searchEdit->setFixedHeight(32);
    toolbar->addWidget(m_searchEdit);
    toolbar->addWidget(makeGap(8));

    // Priority filter
    m_priorityFilter = new QComboBox(toolbar);
    m_priorityFilter->addItem(QStringLiteral("All Priorities"), -1);
    m_priorityFilter->addItem(QStringLiteral("Low"),    static_cast<int>(Task::Priority::Low));
    m_priorityFilter->addItem(QStringLiteral("Medium"), static_cast<int>(Task::Priority::Medium));
    m_priorityFilter->addItem(QStringLiteral("High"),   static_cast<int>(Task::Priority::High));
    m_priorityFilter->setFixedWidth(130);
    m_priorityFilter->setFixedHeight(32);
    toolbar->addWidget(m_priorityFilter);
    toolbar->addWidget(makeGap(8));

    // Tag filter (Item 7)
    m_tagFilter = new QComboBox(toolbar);
    m_tagFilter->addItem(QStringLiteral("All Tags"), QString());
    m_tagFilter->setFixedWidth(130);
    m_tagFilter->setFixedHeight(32);
    toolbar->addWidget(m_tagFilter);
    toolbar->addWidget(makeGap(16));

    // Undo / Redo buttons (Item 10)
    const QString undoRedoStyle = QStringLiteral(
        "QPushButton {"
        "  background: transparent; border: 1px solid #e2e8f0;"
        "  border-radius: 6px; padding: 0 10px;"
        "  color: #4a5568; font-size: 16px; font-weight: 600;"
        "}"
        "QPushButton:hover:enabled { background: #edf2f7; border-color: #90cdf4; }"
        "QPushButton:disabled { color: #cbd5e0; border-color: #edf2f7; }"
    );

    m_undoBtn = new QPushButton(QStringLiteral("\u21B6"), toolbar);  // ↶
    m_undoBtn->setToolTip(QStringLiteral("Undo  (Ctrl+Z)"));
    m_undoBtn->setCursor(Qt::PointingHandCursor);
    m_undoBtn->setFixedHeight(32);
    m_undoBtn->setFixedWidth(36);
    m_undoBtn->setEnabled(false);
    m_undoBtn->setStyleSheet(undoRedoStyle);
    toolbar->addWidget(m_undoBtn);
    toolbar->addWidget(makeGap(4));

    m_redoBtn = new QPushButton(QStringLiteral("\u21B7"), toolbar);  // ↷
    m_redoBtn->setToolTip(QStringLiteral("Redo  (Ctrl+Y)"));
    m_redoBtn->setCursor(Qt::PointingHandCursor);
    m_redoBtn->setFixedHeight(32);
    m_redoBtn->setFixedWidth(36);
    m_redoBtn->setEnabled(false);
    m_redoBtn->setStyleSheet(undoRedoStyle);
    toolbar->addWidget(m_redoBtn);

    // Flexible spacer
    auto *spacer = new QWidget(toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet(QStringLiteral("background: transparent;"));
    toolbar->addWidget(spacer);

    // Add Task button
    auto *addBtn = new QPushButton(QStringLiteral("+ Add Task"), toolbar);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(32);
    addBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #4299e1; color: white; border: none;"
        "  padding: 0 20px; border-radius: 7px;"
        "  font-weight: 600; font-size: 13px;"
        "}"
        "QPushButton:hover { background: #3182ce; }"
        "QPushButton:pressed { background: #2b6cb0; }"
    ));
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddTask);
    toolbar->addWidget(addBtn);
    toolbar->addWidget(makeGap(4));

    // ── Board canvas ─────────────────────────────────────────────────────
    auto *central = new BoardCanvasWidget(this);
    setCentralWidget(central);

    auto *boardLayout = new QHBoxLayout(central);
    boardLayout->setContentsMargins(20, 20, 20, 20);
    boardLayout->setSpacing(16);

    m_columns[0] = new BoardColumnWidget(QStringLiteral("To Do"),      Task::Status::ToDo,       m_board.get(), central);
    m_columns[1] = new BoardColumnWidget(QStringLiteral("In Progress"), Task::Status::InProgress, m_board.get(), central);
    m_columns[2] = new BoardColumnWidget(QStringLiteral("Done"),        Task::Status::Done,        m_board.get(), central);

    for (auto *col : m_columns) {
        col->setMinimumWidth(260);
        boardLayout->addWidget(col, 1);
    }
}

void MainWindow::connectSignals()
{
    // Board signals -> MainWindow slots (for UI rebuild + save)
    connect(m_board.get(), &Board::taskAdded,   this, &MainWindow::onTaskAdded);
    connect(m_board.get(), &Board::taskUpdated, this, &MainWindow::onTaskUpdated);
    connect(m_board.get(), &Board::taskRemoved, this, &MainWindow::onTaskRemoved);
    connect(m_board.get(), &Board::boardReset,  this, &MainWindow::onBoardReset);

    // Filter controls
    if (m_searchEdit)
        connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    if (m_priorityFilter)
        connect(m_priorityFilter, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);
    if (m_tagFilter)
        connect(m_tagFilter, &QComboBox::currentIndexChanged, this, &MainWindow::onFilterChanged);

    // Column card signals
    for (auto *col : m_columns) {
        connect(col, &BoardColumnWidget::editRequested,    this, &MainWindow::onEditRequested);
        connect(col, &BoardColumnWidget::deleteRequested,  this, &MainWindow::onDeleteRequested,  Qt::QueuedConnection);
        connect(col, &BoardColumnWidget::detailsRequested, this, &MainWindow::onDetailsRequested);
        connect(col, &BoardColumnWidget::moveRequested,    this, &MainWindow::onMoveRequested,    Qt::QueuedConnection);
    }

    // Item 10: Undo/Redo button clicks
    connect(m_undoBtn, &QPushButton::clicked, this, [this]() {
        m_history->undo(*m_board);
    });
    connect(m_redoBtn, &QPushButton::clicked, this, [this]() {
        m_history->redo(*m_board);
    });

    // Item 10: CommandHistory signals -> button enabled state
    connect(m_history.get(), &CommandHistory::canUndoChanged, m_undoBtn, &QPushButton::setEnabled);
    connect(m_history.get(), &CommandHistory::canRedoChanged, m_redoBtn, &QPushButton::setEnabled);

    // Item 10: Keyboard shortcuts Ctrl+Z / Ctrl+Y
    auto *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, [this]() {
        if (m_history->canUndo())
            m_history->undo(*m_board);
    });

    auto *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    connect(redoShortcut, &QShortcut::activated, this, [this]() {
        if (m_history->canRedo())
            m_history->redo(*m_board);
    });
}

// Item 7: collect all unique tags and rebuild the dropdown
void MainWindow::rebuildTagFilterCombo()
{
    if (!m_tagFilter) return;

    const QString currentTag = m_tagFilter->currentData().toString();

    m_tagFilter->blockSignals(true);
    m_tagFilter->clear();
    m_tagFilter->addItem(QStringLiteral("All Tags"), QString());

    QSet<QString> seen;
    for (const Task &t : m_board->tasks()) {
        for (const QString &tag : t.tags()) {
            if (!seen.contains(tag)) {
                seen.insert(tag);
                m_tagFilter->addItem(tag, tag);
            }
        }
    }

    const int idx = m_tagFilter->findData(currentTag);
    m_tagFilter->setCurrentIndex(idx >= 0 ? idx : 0);
    m_tagFilter->blockSignals(false);
}

void MainWindow::onFilterChanged()
{
    const QString query = m_searchEdit ? m_searchEdit->text() : QString();

    std::optional<Task::Priority> priority;
    if (m_priorityFilter && m_priorityFilter->currentIndex() > 0)
        priority = static_cast<Task::Priority>(m_priorityFilter->currentData().toInt());

    QStringList tagFilter;
    if (m_tagFilter && !m_tagFilter->currentData().toString().isEmpty())
        tagFilter.append(m_tagFilter->currentData().toString());

    for (auto *col : m_columns) {
        if (col)
            col->setFilter(query, priority, tagFilter);
    }
}

void MainWindow::onTaskAdded(const Task &task)
{
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    rebuildTagFilterCombo();
    onFilterChanged();
    saveBoard();
}

void MainWindow::onTaskUpdated(const Task &task)
{
    for (auto *col : m_columns) col->removeCard(task.id());
    if (auto *col = columnFor(task.status()))
        col->addCard(task);
    rebuildTagFilterCombo();
    onFilterChanged();
    saveBoard();
}

void MainWindow::onTaskRemoved(QUuid id)
{
    for (auto *col : m_columns) col->removeCard(id);
    rebuildTagFilterCombo();
    onFilterChanged();
    saveBoard();
}

void MainWindow::onBoardReset()
{
    m_columns[0]->rebuildAll(m_board->tasksByStatus(Task::Status::ToDo));
    m_columns[1]->rebuildAll(m_board->tasksByStatus(Task::Status::InProgress));
    m_columns[2]->rebuildAll(m_board->tasksByStatus(Task::Status::Done));
    rebuildTagFilterCombo();

    // Item 9: restore last saved filter state after board load
    QSettings settings(QStringLiteral("qt-taskboard"), QStringLiteral("qt-taskboard"));
    if (m_searchEdit)
        m_searchEdit->setText(settings.value(QStringLiteral("filter/text")).toString());
    if (m_priorityFilter) {
        const int pi = settings.value(QStringLiteral("filter/priority"), 0).toInt();
        if (pi >= 0 && pi < m_priorityFilter->count())
            m_priorityFilter->setCurrentIndex(pi);
    }
    if (m_tagFilter) {
        const QString savedTag = settings.value(QStringLiteral("filter/tag")).toString();
        const int ti = m_tagFilter->findData(savedTag);
        m_tagFilter->setCurrentIndex(ti >= 0 ? ti : 0);
    }

    onFilterChanged();
}

void MainWindow::onAddTask()
{
    TaskEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Task t = dlg.task();
        if (!t.title().isEmpty()) {
            // Item 10: route through CommandHistory so this is undoable
            m_history->push(std::make_unique<AddTaskCommand>(t), *m_board);
        }
    }
}

void MainWindow::onEditRequested(QUuid id)
{
    const auto opt = m_board->findTask(id);
    if (!opt.has_value()) return;

    TaskEditDialog dlg(opt.value(), this);
    if (dlg.exec() == QDialog::Accepted) {
        Task updated = dlg.task();
        updated.setStatus(opt.value().status());  // preserve column placement
        // Item 10: undoable update (stores before/after snapshots)
        m_history->push(
            std::make_unique<UpdateTaskCommand>(opt.value(), updated),
            *m_board);
    }
}

void MainWindow::onDeleteRequested(QUuid id)
{
    const auto opt = m_board->findTask(id);
    if (!opt.has_value()) return;
    // Item 10: undoable remove (captures full Task for restore)
    m_history->push(std::make_unique<RemoveTaskCommand>(opt.value()), *m_board);
}

// Item 8: open the read-only task details dialog
void MainWindow::onMoveRequested(QUuid id, Task::Status newStatus)
{
    const auto opt = m_board->findTask(id);
    if (!opt.has_value() || opt.value().status() == newStatus) return;
    m_history->push(
        std::make_unique<MoveTaskCommand>(id, opt.value().status(), newStatus),
        *m_board);
}

void MainWindow::onDetailsRequested(QUuid id)
{
    const auto opt = m_board->findTask(id);
    if (!opt.has_value()) return;
    TaskDetailsDialog dlg(opt.value(), this);
    dlg.exec();
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
