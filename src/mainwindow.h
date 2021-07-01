#pragma once

#include <QMainWindow>
#include <QTreeView>

#include "data/rail/railway.h"
#include "data/diagram/diagram.h"

#include "kernel/diagramwidget.h"
#include "model/diagram/diagramnavimodel.h"

//for SARibbon
#include "SARibbonMainWindow.h"
#include "DockManager.h"

/**
 * @brief The MainWindow class
 */
class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
    Diagram _diagram;
    DiagramWidget* diagramWidget;
    ads::CDockManager* manager;

    //窗口，Model的指针
    DiagramNaviModel* naviModel;
    QTreeView* naviView;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    /**
     * 程序启动，构建UI
     * 注意启动的第一阶段先不管打开运行图的事情
     */
    void initUI();

    void initDockWidgets();

    void initToolbar();

    /**
     * 重置操作之前调用，例如打开运行图 
     */
    void beforeResetGraph();

    /**
     * 重置操作之后调用
     */
    void endResetGraph();

private slots:
    void actNewGraph();
    void actOpenGraph();
    void actSaveGraph();
    void actSaveGraphAs();
};

