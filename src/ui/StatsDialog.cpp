#include "ui/StatsDialog.h"
#include "ui/ChartWidget.h"
#include "core/Board.h"
#include "core/Task.h"

#include <QDate>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QVBoxLayout>

// ── helper ──────────────────────────────────────────────────────────────────

static QLabel *makeStatBox(const QString &value, const QString &label,
                           const QColor &color, QWidget *parent)
{
    auto *box = new QLabel(parent);
    box->setAlignment(Qt::AlignCenter);
    box->setMinimumWidth(110);
    box->setFixedHeight(80);
    box->setText(QStringLiteral(
        "<div style='font-size:28px;font-weight:700;color:%1;'>%2</div>"
        "<div style='font-size:11px;color:#718096;margin-top:2px;'>%3</div>"
    ).arg(color.name(), value, label));
    box->setStyleSheet(QStringLiteral(
        "background: white; border: 1px solid #e2e8f0;"
        "border-radius: 10px; padding: 8px;"
    ));
    return box;
}

// ── StatsDialog ──────────────────────────────────────────────────────────────

StatsDialog::StatsDialog(const Board &board, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Board Statistics"));
    setMinimumWidth(640);
    setStyleSheet(QStringLiteral("background: #f7fafc;"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(20);

    // ── Title ──────────────────────────────────────────────────────────────
    auto *title = new QLabel(QStringLiteral("Board Statistics"), this);
    title->setStyleSheet(QStringLiteral(
        "font-size: 18px; font-weight: 700; color: #1a202c;"
    ));
    root->addWidget(title);

    // ── Gather data ────────────────────────────────────────────────────────
    const QList<Task> allTasks = board.tasks();
    const QDate today = QDate::currentDate();

    int totalTasks    = allTasks.size();
    int doneTasks     = 0;
    int inProgressCnt = 0;
    int overdueCnt    = 0;
    int lowCnt = 0, medCnt = 0, highCnt = 0;
    QMap<QString, int> tagFreq;

    for (const Task &t : allTasks) {
        if (t.status() == Task::Status::Done)         ++doneTasks;
        if (t.status() == Task::Status::InProgress)   ++inProgressCnt;
        if (t.dueDate().isValid() && t.dueDate() < today
                && t.status() != Task::Status::Done)  ++overdueCnt;

        switch (t.priority()) {
            case Task::Priority::Low:    ++lowCnt;  break;
            case Task::Priority::Medium: ++medCnt;  break;
            case Task::Priority::High:   ++highCnt; break;
        }
        for (const QString &tag : t.tags())
            tagFreq[tag]++;
    }

    // ── Summary boxes ──────────────────────────────────────────────────────
    auto *summaryRow = new QHBoxLayout;
    summaryRow->setSpacing(12);
    summaryRow->addWidget(makeStatBox(QString::number(totalTasks),
        QStringLiteral("Total Tasks"),  QColor(0x42, 0x99, 0xe1), this));
    summaryRow->addWidget(makeStatBox(QString::number(doneTasks),
        QStringLiteral("Completed"),    QColor(0x48, 0xbb, 0x78), this));
    summaryRow->addWidget(makeStatBox(QString::number(inProgressCnt),
        QStringLiteral("In Progress"),  QColor(0xed, 0x8b, 0x36), this));
    summaryRow->addWidget(makeStatBox(QString::number(overdueCnt),
        QStringLiteral("Overdue"),      QColor(0xf5, 0x65, 0x65), this));
    root->addLayout(summaryRow);

    // ── Charts row ─────────────────────────────────────────────────────────
    auto *chartsRow = new QHBoxLayout;
    chartsRow->setSpacing(16);

    // Column distribution chart
    auto *colChart = new ChartWidget(this);
    QMap<QString, int> colData;
    colData[QStringLiteral("To Do")]      = (int)board.tasksByStatus(Task::Status::ToDo).size();
    colData[QStringLiteral("In Progress")]= (int)board.tasksByStatus(Task::Status::InProgress).size();
    colData[QStringLiteral("Done")]       = (int)board.tasksByStatus(Task::Status::Done).size();
    colChart->setTitle(QStringLiteral("Tasks by Status"));
    colChart->setData(colData, {
        QColor(0x63, 0xb3, 0xed),
        QColor(0xfc, 0xb5, 0x5c),
        QColor(0x68, 0xd3, 0x91),
    });
    colChart->setStyleSheet(QStringLiteral(
        "background: white; border: 1px solid #e2e8f0; border-radius: 10px;"
    ));

    // Priority distribution chart
    auto *priChart = new ChartWidget(this);
    QMap<QString, int> priData;
    priData[QStringLiteral("Low")]    = lowCnt;
    priData[QStringLiteral("Medium")] = medCnt;
    priData[QStringLiteral("High")]   = highCnt;
    priChart->setTitle(QStringLiteral("Tasks by Priority"));
    priChart->setData(priData, {
        QColor(0x68, 0xd3, 0x91),
        QColor(0xfc, 0xb5, 0x5c),
        QColor(0xf6, 0x87, 0x75),
    });
    priChart->setStyleSheet(QStringLiteral(
        "background: white; border: 1px solid #e2e8f0; border-radius: 10px;"
    ));

    chartsRow->addWidget(colChart, 1);
    chartsRow->addWidget(priChart, 1);
    root->addLayout(chartsRow);

    // ── Top tags ───────────────────────────────────────────────────────────
    if (!tagFreq.isEmpty()) {
        auto *tagTitle = new QLabel(QStringLiteral("Top Tags"), this);
        tagTitle->setStyleSheet(QStringLiteral(
            "font-weight: 600; color: #2d3748; font-size: 13px;"
        ));
        root->addWidget(tagTitle);

        auto *tagsRow = new QHBoxLayout;
        tagsRow->setSpacing(8);
        tagsRow->addStretch();

        // Sort by frequency descending and show top 8
        QList<QPair<int, QString>> sorted;
        for (auto it = tagFreq.cbegin(); it != tagFreq.cend(); ++it)
            sorted.append({it.value(), it.key()});
        std::sort(sorted.rbegin(), sorted.rend());

        static const QStringList tagColors = {
            "#bee3f8", "#c6f6d5", "#fef3c7", "#fed7d7",
            "#e9d8fd", "#b2f5ea", "#fbd38d", "#e2e8f0"
        };
        static const QStringList tagText = {
            "#2b6cb0", "#276749", "#92400e", "#c53030",
            "#553c9a", "#234e52", "#c05621", "#4a5568"
        };
        int ti = 0;
        for (const auto &[cnt, tag] : sorted) {
            if (ti >= 8) break;
            auto *chip = new QLabel(QStringLiteral("%1 <b>%2</b>").arg(tag).arg(cnt), this);
            chip->setStyleSheet(QStringLiteral(
                "background: %1; color: %2; border-radius: 12px;"
                "padding: 4px 12px; font-size: 12px;"
            ).arg(tagColors[ti % tagColors.size()], tagText[ti % tagText.size()]));
            tagsRow->addWidget(chip);
            ++ti;
        }
        tagsRow->addStretch();
        root->addLayout(tagsRow);
    }

    // ── Close button ───────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *closeBtn = new QPushButton(QStringLiteral("Close"), this);
    closeBtn->setFixedWidth(100);
    closeBtn->setFixedHeight(34);
    closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #4299e1; color: white; border: none;"
        "  border-radius: 7px; font-weight: 600; }"
        "QPushButton:hover { background: #3182ce; }"
    ));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
}
