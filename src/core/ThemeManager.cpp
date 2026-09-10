#include "core/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QString>

// ── helpers ───────────────────────────────────────────────────────────────────

static QString resourcePath(Theme theme)
{
    return (theme == Theme::Dark) ? QStringLiteral(":/styles/dark.qss")
                                  : QStringLiteral(":/styles/light.qss");
}

// ── public API ────────────────────────────────────────────────────────────────

void ThemeManager::apply(Theme theme)
{
    QFile f(resourcePath(theme));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("ThemeManager: cannot open %s", qPrintable(resourcePath(theme)));
        return;
    }
    qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    f.close();

    // Persist choice so it survives app restart (Item 9 QSettings integration)
    QSettings settings;
    settings.setValue(QStringLiteral("ui/theme"), toString(theme));
}

Theme ThemeManager::toggle()
{
    Theme next = (current() == Theme::Light) ? Theme::Dark : Theme::Light;
    apply(next);
    return next;
}

Theme ThemeManager::current()
{
    QSettings settings;
    return fromString(settings.value(QStringLiteral("ui/theme"),
                                     QStringLiteral("light")).toString());
}

QString ThemeManager::toString(Theme theme)
{
    return (theme == Theme::Dark) ? QStringLiteral("dark") : QStringLiteral("light");
}

Theme ThemeManager::fromString(const QString &s)
{
    return (s == QStringLiteral("dark")) ? Theme::Dark : Theme::Light;
}
