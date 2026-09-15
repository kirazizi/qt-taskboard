#pragma once

#include <QMap>
#include <QString>
#include <QWidget>
#include <QColor>
#include <QList>

/**
 * ChartWidget -- custom animated bar chart drawn with QPainter.
 *
 * Accepts a QMap<QString, int> label->value dataset and a list of bar colors.
 * On show, bars animate from 0 to their target height via a QTimer-driven
 * progress value (no Qt animation framework needed).
 *
 * No Qt Charts module required.
 *
 * Tier 3 Item 12: Statistics Dashboard.
 */
class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);

    // Set chart data. Keys = bar labels, Values = counts.
    void setData(const QMap<QString, int> &data,
                 const QList<QColor>      &palette = {});

    // Optional chart title shown above the bars.
    void setTitle(const QString &title);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void tickAnimation();

private:
    QMap<QString, int> m_data;
    QList<QColor>      m_palette;
    QString            m_title;
    qreal              m_progress = 0.0; // 0.0 -> 1.0 animation progress
    bool               m_animating = false;
};
