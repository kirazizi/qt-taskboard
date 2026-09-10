#pragma once

#include <QDialog>

class Board;

/**
 * StatsDialog -- read-only statistics dashboard for the active board.
 *
 * Displays:
 *   - Summary numbers: total tasks, completed, overdue, in-progress.
 *   - Bar chart: task count per column (To Do / In Progress / Done).
 *   - Bar chart: task count per priority (Low / Medium / High).
 *   - Top tags by frequency.
 *
 * Drawn using the custom ChartWidget (pure QPainter, no Qt Charts module).
 *
 * Tier 3 Item 12.
 */
class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsDialog(const Board &board, QWidget *parent = nullptr);
};
