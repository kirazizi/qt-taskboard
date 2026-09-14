#include "ui/BoardBarWidget.h"
#include <QTimer>
#include "core/ThemeManager.h"
#include <QEvent>
#include <QMouseEvent>
#include "core/BoardManager.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

BoardBarWidget::BoardBarWidget(BoardManager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
{
    setFixedHeight(40);
    setObjectName(QStringLiteral("boardBar"));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(16, 4, 16, 4);
    m_layout->setSpacing(6);
    m_layout->addStretch();

    // "+" new board button (stays at the right end)
    m_addBtn = new QPushButton(QStringLiteral("+"), this);
    m_addBtn->setToolTip(QStringLiteral("New Board"));
    m_addBtn->setFixedSize(28, 28);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &BoardBarWidget::onAddBoard);
    m_layout->addWidget(m_addBtn);

    refresh();
}

void BoardBarWidget::refresh()
{
    // Remove all existing tab containers
    for (auto *w : m_tabs) {
        m_layout->removeWidget(w);
        w->deleteLater();
    }
    m_tabs.clear();

    const bool isDarkTheme = (ThemeManager::current() == Theme::Dark);
    if (m_addBtn) {
        m_addBtn->setStyleSheet(isDarkTheme ? QStringLiteral(
            "QPushButton {"
            "  background: #1e293b; color: #38bdf8;"
            "  border: 1px solid #334155; border-radius: 6px;"
            "  font-size: 16px; font-weight: 700;"
            "}"
            "QPushButton:hover { background: #243247; border-color: #38bdf8; }"
        ) : QStringLiteral(
            "QPushButton {"
            "  background: #ebf8ff; color: #2b6cb0;"
            "  border: 1px solid #bee3f8; border-radius: 6px;"
            "  font-size: 16px; font-weight: 700;"
            "}"
            "QPushButton:hover { background: #bee3f8; }"
        ));
    }

    const int active = m_manager->activeIndex();
    const int total  = m_manager->count();

    for (int i = 0; i < total; ++i) {
        const QString name     = m_manager->metaAt(i).name;
        const bool    isActive = (i == active);
        const bool    canDelete = (total > 1);  // must keep at least 1 board

        // ── Tab container: [  Board Name  ][x]  ──────────────────────────────
        auto *chip = new QWidget(this);
        chip->setFixedHeight(28);
        chip->setCursor(Qt::ArrowCursor);

        auto *chipLayout = new QHBoxLayout(chip);
        chipLayout->setContentsMargins(0, 0, 0, 0);
        chipLayout->setSpacing(0);

        // Board name button (left part of chip)
        auto *nameBtn = new QPushButton(name, chip);
        nameBtn->setCursor(Qt::PointingHandCursor);
        nameBtn->setFixedHeight(28);
        nameBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        const bool isDark = (ThemeManager::current() == Theme::Dark);

        if (isActive) {
            nameBtn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: %1; color: white;"
                "  border: none;"
                "  border-radius: 6px 0px 0px 6px;"
                "  padding: 0 10px 0 14px; font-weight: 600;"
                "}"
            ).arg(isDark ? QStringLiteral("#0284c7") : QStringLiteral("#4299e1")));
        } else {
            nameBtn->setStyleSheet(isDark ? QStringLiteral(
                "QPushButton {"
                "  background: #1e293b; color: #94a3b8;"
                "  border: 1px solid #334155; border-right: none;"
                "  border-radius: 6px 0px 0px 6px;"
                "  padding: 0 10px 0 14px;"
                "}"
                "QPushButton:hover { background: #243247; color: #f1f5f9; }"
            ) : QStringLiteral(
                "QPushButton {"
                "  background: transparent; color: #4a5568;"
                "  border: 1px solid #e2e8f0; border-right: none;"
                "  border-radius: 6px 0px 0px 6px;"
                "  padding: 0 10px 0 14px;"
                "}"
                "QPushButton:hover { background: #f7fafc; color: #2d3748; }"
            ));
        }

        // Switch board on click
        connect(nameBtn, &QPushButton::clicked, this, [this, i]() {
            m_manager->setActiveIndex(i);
        });

        // Double-click to rename
        nameBtn->installEventFilter(this);

        chipLayout->addWidget(nameBtn);

        // Delete (×) button (right part of chip)
        auto *delBtn = new QPushButton(QStringLiteral("\u00D7"), chip); // ×
        delBtn->setFixedSize(18, 28);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setToolTip(QStringLiteral("Delete this board"));
        delBtn->setVisible(canDelete); // hide when only 1 board remains

        if (isActive) {
            delBtn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: %1; color: rgba(255,255,255,0.85);"
                "  border: none; border-left: 1px solid rgba(255,255,255,0.25);"
                "  border-radius: 0px 6px 6px 0px;"
                "  font-size: 13px; padding: 0;"
                "}"
                "QPushButton:hover { background: %2; color: white; }"
            ).arg(isDark ? QStringLiteral("#0369a1") : QStringLiteral("#3182ce"),
                 isDark ? QStringLiteral("#075985") : QStringLiteral("#2b6cb0")));
        } else {
            delBtn->setStyleSheet(isDark ? QStringLiteral(
                "QPushButton {"
                "  background: #1e293b; color: #64748b;"
                "  border: 1px solid #334155; border-left: none;"
                "  border-radius: 0px 6px 6px 0px;"
                "  font-size: 13px; padding: 0;"
                "}"
                "QPushButton:hover { background: #7f1d1d; color: #fca5a5; border-color: #991b1b; }"
            ) : QStringLiteral(
                "QPushButton {"
                "  background: transparent; color: #a0aec0;"
                "  border: 1px solid #e2e8f0; border-left: none;"
                "  border-radius: 0px 6px 6px 0px;"
                "  font-size: 13px; padding: 0;"
                "}"
                "QPushButton:hover { background: #fff5f5; color: #c53030; border-color: #fed7d7; }"
            ));
        }

        connect(delBtn, &QPushButton::clicked, this, [this, i]() {
            const QString boardName = m_manager->metaAt(i).name;
            const int taskCount = m_manager->boardAt(i)
                                      ? m_manager->boardAt(i)->tasks().size()
                                      : 0;

            // Confirm before deleting non-empty boards
            const QUuid boardId = m_manager->metaAt(i).id;

            // Confirm before deleting non-empty boards
            if (taskCount > 0) {
                const auto answer = QMessageBox::question(
                    this,
                    QStringLiteral("Delete Board"),
                    QStringLiteral(
                        "Delete \"%1\"?\n"
                        "This board has %2 task(s). They will be permanently removed."
                    ).arg(boardName).arg(taskCount),
                    QMessageBox::Yes | QMessageBox::Cancel,
                    QMessageBox::Cancel
                );
                if (answer != QMessageBox::Yes) return;
            }

            // Execute deletion asynchronously on next event loop tick
            // so delBtn's click event finishes cleanly first
            QTimer::singleShot(0, this, [this, boardId]() {
                for (int k = 0; k < m_manager->count(); ++k) {
                    if (m_manager->metaAt(k).id == boardId) {
                        m_manager->removeBoard(k);
                        break;
                    }
                }
            });
        });

        chipLayout->addWidget(delBtn);

        // If not deletable (only board), make name button fully rounded
        if (!canDelete) {
            nameBtn->setStyleSheet(nameBtn->styleSheet()
                .replace(QStringLiteral("border-radius: 6px 0px 0px 6px;"),
                         QStringLiteral("border-radius: 6px;"))
                .replace(QStringLiteral("border-right: none;"), QString())
            );
        }

        m_layout->insertWidget(i, chip);
        m_tabs.append(chip);
    }
}

bool BoardBarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        // Find which tab's nameBtn was double-clicked
        for (int i = 0; i < m_tabs.size(); ++i) {
            // nameBtn is the first child of the chip container
            auto *chip    = m_tabs[i];
            auto *nameBtn = chip->findChild<QPushButton *>();
            if (nameBtn && nameBtn == obj) {
                bool ok = false;
                const QString current = m_manager->metaAt(i).name;
                const QString newName = QInputDialog::getText(
                    this,
                    QStringLiteral("Rename Board"),
                    QStringLiteral("Board name:"),
                    QLineEdit::Normal,
                    current,
                    &ok
                );
                if (ok && !newName.trimmed().isEmpty())
                    m_manager->renameBoard(i, newName.trimmed());
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void BoardBarWidget::onAddBoard()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this,
        QStringLiteral("New Board"),
        QStringLiteral("Board name:"),
        QLineEdit::Normal,
        QStringLiteral("New Board"),
        &ok
    );
    if (ok && !name.trimmed().isEmpty()) {
        const int idx = m_manager->addBoard(name.trimmed());
        m_manager->setActiveIndex(idx);
    }
}
