#include "ui/TaskCardDelegate.h"
#include "core/Task.h"
#include "core/TaskListModel.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>
#include <QUuid>

// ── helpers ─────────────────────────────────────────────────────────────────

static QColor priorityColor(Task::Priority p)
{
    switch (p) {
    case Task::Priority::High:   return QColor(0xfe, 0xd7, 0xd7); // red-100
    case Task::Priority::Medium: return QColor(0xfe, 0xf0, 0xc7); // amber-100
    case Task::Priority::Low:    return QColor(0xc6, 0xf6, 0xd5); // green-100
    }
    return {};
}

static QColor priorityTextColor(Task::Priority p)
{
    switch (p) {
    case Task::Priority::High:   return QColor(0xc5, 0x30, 0x30); // red-700
    case Task::Priority::Medium: return QColor(0x92, 0x40, 0x0e); // amber-700
    case Task::Priority::Low:    return QColor(0x27, 0x67, 0x49); // green-700
    }
    return {};
}

static QString priorityLabel(Task::Priority p)
{
    switch (p) {
    case Task::Priority::High:   return QStringLiteral("High");
    case Task::Priority::Medium: return QStringLiteral("Medium");
    case Task::Priority::Low:    return QStringLiteral("Low");
    }
    return {};
}

// ── Tag pastel palette (cycles through 8 colours) ────────────────────────────
static QColor tagBg(int idx)
{
    static const QColor palette[] = {
        QColor(0xbe, 0xe3, 0xf8), // blue-100
        QColor(0xc6, 0xf6, 0xd5), // green-100
        QColor(0xfe, 0xf0, 0xc7), // amber-100
        QColor(0xfe, 0xd7, 0xd7), // red-100
        QColor(0xe9, 0xd8, 0xfd), // purple-100
        QColor(0xb2, 0xf5, 0xea), // teal-100
        QColor(0xfb, 0xd3, 0x8d), // orange-100
        QColor(0xe2, 0xe8, 0xf0), // gray-100
    };
    return palette[idx % 8];
}
static QColor tagText(int idx)
{
    static const QColor palette[] = {
        QColor(0x2b, 0x6c, 0xb0),
        QColor(0x27, 0x67, 0x49),
        QColor(0x92, 0x40, 0x0e),
        QColor(0xc5, 0x30, 0x30),
        QColor(0x55, 0x3c, 0x9a),
        QColor(0x23, 0x4e, 0x52),
        QColor(0xc0, 0x56, 0x21),
        QColor(0x4a, 0x55, 0x68),
    };
    return palette[idx % 8];
}

// ── TaskCardDelegate ─────────────────────────────────────────────────────────

TaskCardDelegate::TaskCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize TaskCardDelegate::sizeHint(const QStyleOptionViewItem &,
                                  const QModelIndex &) const
{
    return QSize(0, 96); // height per card; width comes from view
}

void TaskCardDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    const QVariant var = index.data(TaskListModel::TaskRole);
    if (!var.isValid()) return;
    const Task task = var.value<Task>();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Card background
    const QRect cardRect = option.rect.adjusted(8, 6, -8, -6);
    QPainterPath path;
    path.addRoundedRect(cardRect, 10, 10);

    // Subtle selection highlight
    if (option.state & QStyle::State_Selected) {
        painter->fillPath(path, QColor(0xeb, 0xf8, 0xff));
        painter->setPen(QPen(QColor(0x90, 0xcd, 0xf4), 1.5));
    } else {
        painter->fillPath(path, Qt::white);
        painter->setPen(QPen(QColor(0xe2, 0xe8, 0xf0), 1));
    }
    painter->drawPath(path);

    // ── Content layout ──────────────────────────────────────────────────────
    const int pad = 12;
    int curY = cardRect.top() + pad;
    const int contentW = cardRect.width() - pad * 2;

    QFont boldFont = option.font;
    boldFont.setWeight(QFont::DemiBold);
    boldFont.setPointSizeF(option.font.pointSizeF());
    QFont smallFont = option.font;
    smallFont.setPointSizeF(option.font.pointSizeF() * 0.88);

    // Title
    painter->setFont(boldFont);
    painter->setPen(QColor(0x1a, 0x20, 0x2c));
    const QString elidedTitle = QFontMetrics(boldFont).elidedText(
        task.title(), Qt::ElideRight, contentW);
    painter->drawText(cardRect.left() + pad, curY,
                      contentW, QFontMetrics(boldFont).height(),
                      Qt::AlignLeft | Qt::AlignTop, elidedTitle);
    curY += QFontMetrics(boldFont).height() + 6;

    // Priority badge + tags row
    const int pillH = 18;
    const int pillRadius = 9;
    int x = cardRect.left() + pad;

    // Priority badge
    painter->setFont(smallFont);
    const QString priLabel = priorityLabel(task.priority());
    const int priW = QFontMetrics(smallFont).horizontalAdvance(priLabel) + 16;
    QPainterPath pill;
    pill.addRoundedRect(QRectF(x, curY, priW, pillH), pillRadius, pillRadius);
    painter->fillPath(pill, priorityColor(task.priority()));
    painter->setPen(priorityTextColor(task.priority()));
    painter->drawText(QRect(x, curY, priW, pillH),
                      Qt::AlignCenter, priLabel);
    x += priW + 6;

    // Tag pills
    int tagIdx = 0;
    for (const QString &tag : task.tags()) {
        const int tagW = QFontMetrics(smallFont).horizontalAdvance(tag) + 14;
        if (x + tagW > cardRect.right() - pad) break; // don't overflow card
        QPainterPath tp;
        tp.addRoundedRect(QRectF(x, curY, tagW, pillH), pillRadius, pillRadius);
        painter->fillPath(tp, tagBg(tagIdx));
        painter->setPen(tagText(tagIdx));
        painter->drawText(QRect(x, curY, tagW, pillH), Qt::AlignCenter, tag);
        x += tagW + 6;
        ++tagIdx;
    }
    curY += pillH + 6;

    // Due date (right-aligned, small, muted)
    if (task.dueDate().isValid()) {
        painter->setFont(smallFont);
        painter->setPen(QColor(0xa0, 0xae, 0xc0));
        const QString dateStr = task.dueDate().toString(QStringLiteral("MMM d, yyyy"));
        painter->drawText(
            QRect(cardRect.left() + pad, curY, contentW, QFontMetrics(smallFont).height()),
            Qt::AlignRight | Qt::AlignTop,
            dateStr
        );
    }

    painter->restore();
}

bool TaskCardDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    Q_UNUSED(model)
    Q_UNUSED(option)

    if (event->type() == QEvent::MouseButtonDblClick) {
        const QVariant var = index.data(TaskListModel::TaskRole);
        if (var.isValid()) {
            const Task task = var.value<Task>();
            emit detailsRequested(task.id());
            return true;
        }
    }
    return false;
}
