#include "ui/TaskDetailsDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>

// ── Helpers ──────────────────────────────────────────────────────────────────

static QLabel *makeFieldLabel(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(text, parent);
    l->setStyleSheet(QStringLiteral(
        "color: #718096; font-size: 11px; font-weight: 600;"
        " background: transparent; text-transform: uppercase;"
        " letter-spacing: 0.5px;"
    ));
    return l;
}

static QLabel *makeValueLabel(const QString &text, QWidget *parent, bool multiline = false)
{
    auto *l = new QLabel(text.isEmpty() ? QStringLiteral("—") : text, parent);
    l->setStyleSheet(QStringLiteral(
        "color: #2d3748; font-size: 13px; background: transparent;"
    ));
    if (multiline) {
        l->setWordWrap(true);
        l->setTextFormat(Qt::PlainText);
    }
    return l;
}

// ── Constructor ──────────────────────────────────────────────────────────────

TaskDetailsDialog::TaskDetailsDialog(const Task &task, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setModal(true);
    setFixedWidth(440);
    buildUi(task);
}

void TaskDetailsDialog::buildUi(const Task &task)
{
    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background: #ffffff;"
        "  border: 1px solid #cbd5e1;"
        "  border-radius: 12px;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(0);

    // ── Header ────────────────────────────────────────────────────────────
    auto *headerRow = new QWidget(this);
    headerRow->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    auto *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 12);

    auto *headerLabel = new QLabel(QStringLiteral("Task Details"), headerRow);
    headerLabel->setStyleSheet(QStringLiteral(
        "color: #1a202c; font-size: 17px; font-weight: 700;"
        " background: transparent; border: none;"
    ));

    auto *closeBtn = new QPushButton(QStringLiteral("\u2715"), headerRow);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: transparent; border: none;"
        "  color: #a0aec0; font-size: 13px; font-weight: 700;"
        "  border-radius: 6px;"
        "}"
        "QPushButton:hover { background: #edf2f7; color: #4a5568; }"
        "QPushButton:pressed { background: #e2e8f0; }"
    ));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    headerLayout->addWidget(headerLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    mainLayout->addWidget(headerRow);

    // ── Divider ──────────────────────────────────────────────────────────
    auto *topDiv = new QWidget(this);
    topDiv->setFixedHeight(1);
    topDiv->setStyleSheet(QStringLiteral("background: #e2e8f0; border: none;"));
    mainLayout->addWidget(topDiv);
    mainLayout->addSpacing(16);

    // ── Fields ───────────────────────────────────────────────────────────
    auto addField = [&](const QString &label, const QString &value, bool multiline = false) {
        mainLayout->addWidget(makeFieldLabel(label, this));
        mainLayout->addSpacing(3);
        mainLayout->addWidget(makeValueLabel(value, this, multiline));
        mainLayout->addSpacing(14);
    };

    addField(QStringLiteral("TITLE"), task.title());
    addField(QStringLiteral("DESCRIPTION"), task.description(), true);

    // Priority with colored pill
    QString priorityText;
    QString priorityStyle;
    switch (task.priority()) {
        case Task::Priority::Low:
            priorityText  = QStringLiteral("Low");
            priorityStyle = QStringLiteral(
                "background: #f0fff4; color: #38a169;"
                " border-radius: 4px; font-size: 12px; font-weight: 600;"
                " padding: 3px 10px; border: 1px solid #c6f6d5;"
            );
            break;
        case Task::Priority::Medium:
            priorityText  = QStringLiteral("Medium");
            priorityStyle = QStringLiteral(
                "background: #fffbeb; color: #d97706;"
                " border-radius: 4px; font-size: 12px; font-weight: 600;"
                " padding: 3px 10px; border: 1px solid #fde68a;"
            );
            break;
        case Task::Priority::High:
            priorityText  = QStringLiteral("High");
            priorityStyle = QStringLiteral(
                "background: #fff5f5; color: #e53e3e;"
                " border-radius: 4px; font-size: 12px; font-weight: 600;"
                " padding: 3px 10px; border: 1px solid #fed7d7;"
            );
            break;
    }
    mainLayout->addWidget(makeFieldLabel(QStringLiteral("PRIORITY"), this));
    mainLayout->addSpacing(3);
    auto *prioLabel = new QLabel(priorityText, this);
    prioLabel->setStyleSheet(priorityStyle);
    prioLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->addWidget(prioLabel);
    mainLayout->addSpacing(14);

    addField(QStringLiteral("DUE DATE"),
             task.dueDate().isValid()
                 ? task.dueDate().toString(QStringLiteral("MMMM d, yyyy"))
                 : QString());

    // Tags row with pill badges
    mainLayout->addWidget(makeFieldLabel(QStringLiteral("TAGS"), this));
    mainLayout->addSpacing(3);
    if (task.tags().isEmpty()) {
        mainLayout->addWidget(makeValueLabel(QString(), this));
    } else {
        auto *tagsRow = new QWidget(this);
        tagsRow->setStyleSheet(QStringLiteral("background: transparent;"));
        auto *tagsLayout = new QHBoxLayout(tagsRow);
        tagsLayout->setContentsMargins(0, 0, 0, 0);
        tagsLayout->setSpacing(6);

        // Deterministic pastel colors from tag hash
        static const QStringList kTagColors = {
            QStringLiteral("background:#ebf8ff;color:#3182ce;border:1px solid #bee3f8;"),
            QStringLiteral("background:#f0fff4;color:#276749;border:1px solid #9ae6b4;"),
            QStringLiteral("background:#faf5ff;color:#6b46c1;border:1px solid #d6bcfa;"),
            QStringLiteral("background:#fff5f5;color:#c53030;border:1px solid #feb2b2;"),
            QStringLiteral("background:#fffbeb;color:#b7791f;border:1px solid #fbd38d;"),
            QStringLiteral("background:#e6fffa;color:#234e52;border:1px solid #81e6d9;"),
        };

        for (const QString &tag : task.tags()) {
            auto *badge = new QLabel(tag, tagsRow);
            const int colorIdx = static_cast<int>(qHash(tag) % static_cast<uint>(kTagColors.size()));
            badge->setStyleSheet(
                kTagColors[colorIdx] +
                QStringLiteral(" border-radius: 4px; font-size: 11px;"
                               " font-weight: 600; padding: 2px 8px;"));
            tagsLayout->addWidget(badge);
        }
        tagsLayout->addStretch();
        mainLayout->addWidget(tagsRow);
    }
    mainLayout->addSpacing(14);

    // Timestamps (Item 8)
    auto *tsDiv = new QWidget(this);
    tsDiv->setFixedHeight(1);
    tsDiv->setStyleSheet(QStringLiteral("background: #f0f4f8; border: none;"));
    mainLayout->addWidget(tsDiv);
    mainLayout->addSpacing(12);

    const QString dtFmt = QStringLiteral("MMM d, yyyy  hh:mm");
    addField(QStringLiteral("CREATED"),
             task.createdAt().isValid()
                 ? task.createdAt().toString(dtFmt)
                 : QStringLiteral("unknown"));
    addField(QStringLiteral("LAST MODIFIED"),
             task.modifiedAt().isValid()
                 ? task.modifiedAt().toString(dtFmt)
                 : QStringLiteral("unknown"));

    // ── Close button ─────────────────────────────────────────────────────
    auto *bottomDiv = new QWidget(this);
    bottomDiv->setFixedHeight(1);
    bottomDiv->setStyleSheet(QStringLiteral("background: #e2e8f0; border: none;"));
    mainLayout->addWidget(bottomDiv);
    mainLayout->addSpacing(12);

    auto *btnRow = new QHBoxLayout;
    auto *closeBtn2 = new QPushButton(QStringLiteral("Close"), this);
    closeBtn2->setCursor(Qt::PointingHandCursor);
    closeBtn2->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #4299e1; border: none;"
        "  border-radius: 7px; padding: 8px 28px;"
        "  color: white; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #3182ce; }"
        "QPushButton:pressed { background: #2b6cb0; }"
    ));
    connect(closeBtn2, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn2);
    mainLayout->addLayout(btnRow);
}

void TaskDetailsDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void TaskDetailsDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void TaskDetailsDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}

void TaskDetailsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void TaskDetailsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && isMinimized()) {
        accept();
    }
    QDialog::changeEvent(event);
}
