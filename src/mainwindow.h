#pragma once

#include <QMainWindow>

#include "data/rail/railway.h"
#include "data/diagram/diagram.h"

#include "kernel/diagramwidget.h"

//for SARibbon
#include "SARibbonMainWindow.h"
#include "DockManager.h"

/**
 * @brief The MainWindow class
 * 这个暂时只用来测试
 */
class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
    Diagram _diagram;
    DiagramWidget* diagramWidget;
    ads::CDockManager* manager;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

