#include "ui/ChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(180);
}

void ChartWidget::setData(const QMap<QString, int> &data,
                          const QList<QColor>      &palette)
{
    m_data    = data;
    m_palette = palette;
    m_progress = 0.0;
    update();
}

void ChartWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

QSize ChartWidget::sizeHint() const
{
    return QSize(320, 200);
}

void ChartWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // Kick off animation when widget becomes visible
    m_progress  = 0.0;
    m_animating = true;
    auto *timer = new QTimer(this);
    timer->setInterval(16); // ~60 fps
    connect(timer, &QTimer::timeout, this, &ChartWidget::tickAnimation);
    connect(timer, &QTimer::timeout, this, [timer, this]() {
        if (!m_animating) {
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start();
}

void ChartWidget::tickAnimation()
{
    m_progress += 0.05;
    if (m_progress >= 1.0) {
        m_progress  = 1.0;
        m_animating = false;
    }
    update();
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    if (m_data.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W   = width();
    const int H   = height();
    const int n   = m_data.size();

    // Title
    int titleH = 0;
    if (!m_title.isEmpty()) {
        QFont titleFont = font();
        titleFont.setPointSizeF(font().pointSizeF() * 1.1);
        titleFont.setWeight(QFont::DemiBold);
        p.setFont(titleFont);
        p.setPen(QColor(0x2d, 0x37, 0x48));
        const int tw = QFontMetrics(titleFont).horizontalAdvance(m_title);
        p.drawText((W - tw) / 2, 18, m_title);
        titleH = 28;
    }

    const int labelH  = 22;
    const int padTop  = titleH + 12;
    const int padBot  = labelH + 8;
    const int padSide = 20;

    const int chartH = H - padTop - padBot;
    const int chartW = W - padSide * 2;
    if (chartH <= 0 || chartW <= 0) return;

    // Max value for scaling
    int maxVal = 1;
    for (auto v : m_data.values())
        maxVal = std::max(maxVal, v);

    const int   barW   = std::max(24, (chartW - (n + 1) * 8) / n);
    const int   gap    = (chartW - n * barW) / (n + 1);
    QFont       lblFont = font();
    QFontMetrics fm(lblFont);

    // Default pastel palette
    static const QList<QColor> defaultPalette = {
        QColor(0x63, 0xb3, 0xed), // blue
        QColor(0x68, 0xd3, 0x91), // green
        QColor(0xfc, 0xb5, 0x5c), // amber
        QColor(0xf6, 0x87, 0x75), // red-orange
        QColor(0xb7, 0x94, 0xf4), // purple
        QColor(0x76, 0xe4, 0xf7), // cyan
    };
    const QList<QColor> &pal = m_palette.isEmpty() ? defaultPalette : m_palette;

    // Draw baseline
    p.setPen(QPen(QColor(0xe2, 0xe8, 0xf0), 1));
    p.drawLine(padSide, H - padBot, W - padSide, H - padBot);

    int i = 0;
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it, ++i) {
        const int x     = padSide + gap + i * (barW + gap);
        const int val   = it.value();
        const qreal pct = (maxVal > 0) ? (static_cast<qreal>(val) / maxVal) : 0.0;
        const int barH  = static_cast<int>(chartH * pct * m_progress);
        const int y     = H - padBot - barH;

        // Bar with rounded top
        const QColor barColor = pal[i % pal.size()];
        QPainterPath path;
        const int r = std::min(6, barW / 3);
        path.addRoundedRect(QRectF(x, y, barW, barH), r, r);
        // Square off bottom corners
        if (barH > r) {
            path.addRect(QRectF(x, y + barH - r, barW, r));
        }
        p.fillPath(path, barColor);

        // Value label above bar (only when animation near complete)
        if (m_progress > 0.85 && val > 0) {
            const QString valStr = QString::number(val);
            p.setPen(QColor(0x2d, 0x37, 0x48));
            p.setFont(lblFont);
            const int tw = fm.horizontalAdvance(valStr);
            p.drawText(x + (barW - tw) / 2, y - 4, valStr);
        }

        // Key label below bar
        const QString key = it.key();
        const int kw = fm.horizontalAdvance(key);
        p.setPen(QColor(0x71, 0x80, 0x96));
        p.setFont(lblFont);
        p.drawText(x + (barW - kw) / 2, H - 6, key);
    }
}
