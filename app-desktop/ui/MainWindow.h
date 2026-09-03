#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //void on_btnTestSound_clicked();

    //void on_btnStartStop_clicked();

private:
    Ui::MainWindowClass ui;
};

