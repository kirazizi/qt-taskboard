#pragma once

#include <QMainWindow>

/**
 * MainWindow -- top-level application window.
 *
 * Ownership: Qt parent-child tree owns all child widgets.
 * MainWindow itself is stack-allocated in main.cpp.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
};
