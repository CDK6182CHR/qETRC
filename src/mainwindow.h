#pragma once

#include <QMainWindow>

#include "data/rail/railway.h"
#include "data/diagram/diagram.h"

#include "kernel/diagramwidget.h"


/**
 * @brief The MainWindow class
 * 这个暂时只用来测试
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    Diagram _diagram;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

