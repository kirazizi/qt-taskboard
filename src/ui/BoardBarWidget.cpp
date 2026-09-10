#include "ui/BoardBarWidget.h"
#include <QEvent>
#include <QMouseEvent>
#include "core/BoardManager.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QPushButton>
#include <QMessageBox>

BoardBarWidget::BoardBarWidget(BoardManager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
{
    setFixedHeight(40);
    setStyleSheet(QStringLiteral(
        "background: rgba(255,255,255,0.85);"
        "border-bottom: 1px solid #e2e8f0;"
    ));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(16, 4, 16, 4);
    m_layout->setSpacing(6);
    m_layout->addStretch();

    // "+" new board button (stays at the end)
    auto *addBtn = new QPushButton(QStringLiteral("+"), this);
    addBtn->setToolTip(QStringLiteral("New Board"));
    addBtn->setFixedSize(28, 28);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #ebf8ff; color: #2b6cb0;"
        "  border: 1px solid #bee3f8; border-radius: 6px;"
        "  font-size: 16px; font-weight: 700;"
        "}"
        "QPushButton:hover { background: #bee3f8; }"
    ));
    connect(addBtn, &QPushButton::clicked, this, &BoardBarWidget::onAddBoard);
    m_layout->addWidget(addBtn);

    refresh();
}

void BoardBarWidget::refresh()
{
    // Remove all existing tab buttons (leave stretch + add-button)
    for (auto *btn : m_tabs) {
        m_layout->removeWidget(btn);
        btn->deleteLater();
    }
    m_tabs.clear();

    const int active = m_manager->activeIndex();
    const int total  = m_manager->count();

    // Insert tabs before the stretch (index 0 is stretch, so insert at front)
    for (int i = 0; i < total; ++i) {
        const QString name = m_manager->metaAt(i).name;
        const bool isActive = (i == active);

        auto *tab = new QPushButton(name, this);
        tab->setCursor(Qt::PointingHandCursor);
        tab->setFixedHeight(28);
        tab->setCheckable(true);
        tab->setChecked(isActive);

        if (isActive) {
            tab->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: #4299e1; color: white;"
                "  border: none; border-radius: 6px;"
                "  padding: 0 14px; font-weight: 600;"
                "}"
            ));
        } else {
            tab->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: transparent; color: #4a5568;"
                "  border: 1px solid #e2e8f0; border-radius: 6px;"
                "  padding: 0 14px;"
                "}"
                "QPushButton:hover { background: #f7fafc; color: #2d3748; }"
            ));
        }

        // Switch board on click
        connect(tab, &QPushButton::clicked, this, [this, i]() {
            m_manager->setActiveIndex(i);
        });

        // Double-click to rename
        tab->installEventFilter(this);

        m_layout->insertWidget(i, tab);
        m_tabs.append(tab);
    }
}

bool BoardBarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i] == obj) {
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
