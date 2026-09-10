#pragma once

#include <QString>

/**
 * ThemeManager -- applies application-wide QSS themes at runtime.
 *
 * Design choices:
 *   - All methods are static; no instance needed.
 *   - Lives in core/ but uses QApplication (Qt Core), not QWidget.
 *   - Theme preference is read/written via QSettings key "ui/theme".
 *   - The two bundled themes are embedded in resources.qrc:
 *       :/styles/light.qss  (default -- existing frosted glass theme)
 *       :/styles/dark.qss   (deep navy/slate dark mode)
 *
 * Tier 3 Item 14: Runtime dark/light theme switching.
 */

enum class Theme {
    Light,
    Dark
};

class ThemeManager
{
public:
    // Apply the given theme to the whole application immediately.
    // Loads the matching QSS from Qt resources and calls qApp->setStyleSheet().
    static void apply(Theme theme);

    // Toggle between Light and Dark and return the new theme.
    static Theme toggle();

    // Returns the currently active theme (reads QSettings).
    static Theme current();

    // Human-readable string for a theme value.
    static QString toString(Theme theme);
    static Theme   fromString(const QString &s);
};
