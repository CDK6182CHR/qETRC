#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QList>
#include <QUndoStack>
#include <QStringLiteral>

#include "data/rail/railway.h"
#include "data/diagram/diagram.h"

#include "kernel/diagramwidget.h"
#include "model/diagram/diagramnavimodel.h"
#include "editors/trainlistwidget.h"
#include "navi/navitree.h"

//for SARibbon
#include "SARibbonMainWindow.h"
#include "SARibbonMenu.h"
#include "SARibbonContextCategory.h"

#include "DockManager.h"

#include "traincontext.h"

/**
 * @brief The MainWindow class
 */
class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
    Diagram _diagram;
    ads::CDockManager* manager;
    QUndoStack* undoStack;

    //窗口，Model的指针
    DiagramNaviModel* naviModel;
    NaviTree* naviView;
    SARibbonMenu* pageMenu;
    QList<ads::CDockWidget*> diagramDocks;
    QList<DiagramWidget*> diagramWidgets;
    ads::CDockWidget* naviDock, * trainListDock;
    TrainListWidget* trainListWidget;

    SARibbonContextCategory* contextPage;
    TrainContext* contextTrain;

    bool changed = false;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void undoRemoveTrains(const QList<std::shared_ptr<Train>>& trains,
        const QList<int>& indexes);

    void redoRemoveTrains(const QList<std::shared_ptr<Train>>& trains,
        const QList<int>& indexes);

    /**
     * 注意既然是Undo，这个就肯定在栈顶，直接删除最后一个就好了
     */
    void undoAddPage(std::shared_ptr<DiagramPage> page);

    /**
     * 调用前，应当已经把Page加入到Diagram中
     */
    void addPageWidget(std::shared_ptr<DiagramPage> page);

private:
    /**
     * 程序启动，构建UI
     * 注意启动的第一阶段先不管打开运行图的事情
     */
    void initUI();

    void initDockWidgets();

    void initToolbar();

    /**
     * 程序启动，依次尝试读取上次打开的和默认文件
     */
    void loadInitDiagram();

    /**
     * 打开新的运行图、重置运行图等之前执行。
     * 询问是否保存、
     * 清理当前运行图的东西。
     */
    bool clearDiagram();

    /**
     * 清理运行图数据。
     * 用在打开运行图失败时
     */
    void clearDiagramUnchecked();

    /**
     * 重置操作之前调用，例如打开运行图 
     */
    void beforeResetGraph();

    /**
     * 重置操作之后调用
     */
    void endResetGraph();

    /**
     * 打开新运行图时的操作
     * 每个Page添加一个窗口
     */
    void resetDiagramPages();

    /**
     * 打开运行图 返回是否成功
     */
    bool openGraph(const QString& filename);

    /**
     * 向所有运行图窗口添加指定运行线。
     * （简单封装）
     */
    void addTrainLine(Train& train);
    void removeTrainLine(Train& train);

    void updateWindowTitle();


private slots:
    /**
     * act前缀表示action，强调用户直接动作
     */
    void actNewGraph();
    void actOpenGraph();
    void actSaveGraph();
    void actSaveGraphAs();

    

    /**
     * 用户操作的新增运行图页面，需做Undo操作
     */
    void actAddPage(std::shared_ptr<DiagramPage> page);

    void markChanged();

    void markUnchanged();

    /**
     * 引起contextMenu展示或者隐藏的操作
     */
    void focusInPage(std::shared_ptr<DiagramPage> page);
    void focusOutPage();
    void focusInTrain(std::shared_ptr<Train> train);
    void focusOutTrain();
    

public slots:

    /**
     * 从TrainListWidget发起的删除列车操作
     */
    void trainsRemoved(const QList<std::shared_ptr<Train>>& trains, const QList<int>& indexes);

    /**
     * 列车列表变化（添加或删除），提示相关更新
     */
    void informTrainListChanged();

    void informPageListChanged();

    //列车排序。注意不支持撤销！
    void trainsReordered();
};

