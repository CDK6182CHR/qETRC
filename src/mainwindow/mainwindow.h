#pragma once

#ifndef QETRC_MOBILE_2
#include <QList>

#include "kernel/diagramwidget.h"
#include "SARibbonMainWindow.h"
#include "data/diagram/diagram.h"

class SARibbonMenu;
class RoutingContext;
class RulerContext;
class PageContext;
class ViewCategory;
class RailContext;
class TrainContext;
class QSpinBox;
class QUndoView;
class TimetableQuickWidget;
class TrainInfoWidget;
class QUndoStack;
class DiagramNaviModel;
class NaviTree;
class DiagramWidget;
class TrainListWidget;
class RailStationWidget;
class RoutingWidget;
struct ChangeStationNameData;
class LocateDialog;
class RailDBContext;
namespace ads{
class CDockManager;
class CDockAreaWidget;
class CDockWidget;
}
class  GreedyPaintWizard;
class SARibbonActionsManager;

/**
 * @brief The MainWindow class
 */
class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
    Diagram _diagram;
    ads::CDockManager* manager;
    QUndoStack* undoStack;
    QUndoView* undoView;

    //窗口，Model的指针
    DiagramNaviModel* naviModel;
    NaviTree* naviView;
    SARibbonMenu* pageMenu, * railMenu, * appMenu, * forbidMenu, * routingMenu;
    QList<ads::CDockWidget*> diagramDocks;
    QList<DiagramWidget*> diagramWidgets;
    ads::CDockWidget* naviDock, * trainListDock, * routingDock, * timetableQuickDock;
    ads::CDockWidget* trainInfoDock;
    ads::CDockWidget* undoDock, * centralDock;
    QWidget* centralOccupyWidget;
    ads::CDockAreaWidget* centralArea = nullptr;
    TrainListWidget* trainListWidget;
    QList<RailStationWidget*> railStationWidgets;
    QList<ads::CDockWidget*> railStationDocks;
    RoutingWidget* routingWidget;
    TimetableQuickWidget* timetableQuickWidget;
    TrainInfoWidget* trainInfoWidget;

    PageContext* contextPage;
    TrainContext* contextTrain;
    RailContext* contextRail;
    RulerContext* contextRuler;
    RoutingContext* contextRouting;
    RailDBContext* contextDB;
    ViewCategory* catView;

    QSpinBox* spPassedStations;
    LocateDialog* locateDialog = nullptr;
    GreedyPaintWizard* greedyWidget = nullptr;

    /**
     * 可能在多个地方使用到的action，包装一下
     */
    struct {
        QAction* open, * save, * saveas, * newfile;
    } sharedActions;
    QList<QAction*> actRecent;

    friend class ViewCategory;
    friend class PageContext;
    friend class RailContext;
    friend class RoutingContext;
    friend class TrainContext;
    friend class RailDBContext;

    bool changed = false;

    DiagramWidget::SharedActions diaActions;

    SARibbonActionsManager* actMgr;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * 2022.02.07  这个接口尽量不要用
     */
    auto& diagram() { return _diagram; }


    /**
     * 注意既然是Undo，这个就肯定在栈顶，直接删除最后一个就好了
     */
    //void undoAddPage(std::shared_ptr<DiagramPage> page);

    /**
     * 调用前，应当已经把Page加入到Diagram中
     */
    void addPageWidget(std::shared_ptr<DiagramPage> page);

    /**
     * 插入操作，用来撤销删除。
     * 注意dock与diagram的顺序应当完全一致！！
     */
    void insertPageWidget(std::shared_ptr<DiagramPage> page, int index);

    auto* getManager() { return manager; }

    auto* getUndoStack() { return undoStack; }

    auto* getRulerContext() { return contextRuler; }
    auto* getRailContext() { return contextRail; }
    auto* getViewCategory() { return catView; }
    auto* getRoutingMenu() { return routingMenu; }

    /**
     * 列车信息变化，更新指定车次的所有运行线
     * adps: 旧的Adapter，用来做索引删除以前的运行线
     * 右值引用似乎不能作为slot，因此不做slot
     */
    void updateTrainLines(std::shared_ptr<Train> train,
        QVector<std::shared_ptr<TrainAdapter>>&& adps);

    /**
     * 在不进行重新绑定的情况下，更新列车运行线
     */
    void repaintTrainLines(std::shared_ptr<Train> train);

private:
    /**
     * 程序启动，构建UI
     * 注意启动的第一阶段先不管打开运行图的事情
     */
    void initUI();

    void initDockWidgets();

    void initToolbar();

    void initAppMenu();

    /**
     * 程序启动，依次尝试读取上次打开的和默认文件
     */
    void loadInitDiagram(const QString& cmdFile);

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

    

    void updateWindowTitle();

    /**
     * 询问是否保存。返回程序是否继续
     */
    bool saveQuestion();

    /**
     * 2022.09.11 检查新打开的文件，文件已经进入diagram。
     */
    bool checkOpenFile();

    

protected:
    virtual void closeEvent(QCloseEvent* e)override;

    /**
     * 从操作系统拖入文件直接打开
     * c.f. https://blog.csdn.net/weixin_42887343/article/details/118302641
     */
    virtual void dragEnterEvent(QDragEnterEvent* e)override;

    virtual void dropEvent(QDropEvent* e)override;

private slots:
    /**
     * act前缀表示action，强调用户直接动作
     */
    void actNewGraph();
    void actOpenGraph();
    void actSaveGraph();
    void actSaveGraphAs();
    void actPopupAppButton();
    void addRecentFile(const QString& filename);
    void resetRecentActions();
    void openRecentFile();
    void openFileChecked(const QString& filename);
    

    /**
     * 用户操作的新增运行图页面，需做Undo操作
     * 删除---undo在navi里面搞！
     */
    //void actAddPage(std::shared_ptr<DiagramPage> page);

    void removePageAt(int index);

    /**
     * 用来撤销添加Page
     */
    //void removeLastPage();

    void markChanged();

    void markUnchanged();

    /**
     * 引起contextMenu展示或者隐藏的操作
     */
    void focusInPage(std::shared_ptr<DiagramPage> page);
    void focusOutPage();
    void focusInTrain(std::shared_ptr<Train> train);
    void focusOutTrain();
    void focusInRailway(std::shared_ptr<Railway> rail);
    void focusOutRailway();
    void focusInRuler(std::shared_ptr<Ruler> ruler);
    void focusOutRuler();
    void focusInRouting(std::shared_ptr<Routing> routing);
    void focusOutRouting();

    void undoRemoveTrains(const QList<std::shared_ptr<Train>>& trains);
    void redoRemoveTrains(const QList<std::shared_ptr<Train>>& trains);

    void removeTrain(std::shared_ptr<Train> train);
    void undoRemoveTrain(std::shared_ptr<Train> train);

    /**
     * 线路站表变化时调用这里。
     * 目前的主要任务是重新铺画有关运行图。
     */
    void onStationTableChanged(std::shared_ptr<Railway> rail, bool equiv);

   

    /**
     * 重新铺画所有运行图。
     */
    void updateAllDiagrams();

    void updatePageDiagram(std::shared_ptr<DiagramPage> pg);

    /**
     * 导入车次。刷新相关面板，重新铺画运行图。
     */
    void onTrainsImported();

    /**
     * 删除所有车次
     */
    void removeAllTrains();

    void removeAllTrainsAndRoutings();

    /**
     * 删除新增的线路，只要关闭/删除相应的widget即可
     */
    void undoAddNewRailway(std::shared_ptr<Railway> rail);

    void onNewTrainAdded(std::shared_ptr<Train> train);

    void undoAddNewTrain(std::shared_ptr<Train> train);

    //[[deprecated]]
    //void onActRailwayRemoved(std::shared_ptr<Railway> rail);

    void onActRailwayRemovedU(std::shared_ptr<Railway> rail);

    void removeRailStationWidget(std::shared_ptr<Railway> rail);

    void removeRailStationWidgetAt(int i);

    /**
     * 从工具栏，要求打开新的线路编辑窗口
     */
    void actOpenNewRailWidget();

    /**
     * 删除线路后，有的Page可能同时被删除。此函数检查是否存在这样的页面，
     * 将它们从List中删除。
     * 注意，利用Page的下标一致性约定
     */
    void checkPagesValidity();
    
    /**
     * 更改跨越站数的压栈操作。
     * cmd写在ConfigDialog.h/cpp中
     */
    void actChangePassedStations();

    void showHelpDialog();

    void showAboutDialog();

    void useWpsStyle();
    void useOfficeStyle();

    /**
     * 自定义Ribbon界面
     * @see SARibbon demo
     */
    void actCustomizeRibbon();

    void actRulerPaint();

#ifdef QETRC_GREEDYPAINT_TEST
    void actGreedyPaintFast();
#endif
    void actGreedyPaint();

    void actExitGreedyPaint();

    /**
     * 向所有运行图窗口添加指定运行线。
     * （简单封装）
     */
    void addTrainLine(Train& train);
    void removeTrainLine(const Train& train);

    void actChangeStationName();

    void actNaviToRuler();

    void actNaviToForbid();

    void actToSingleFile();

    void actBatchCopyTrain();

    /**
     * 按列车时刻表重置始发终到站。
     * 将始发终到站分别设为列车时刻表首站和末站。
     */
    void actResetStartingTerminalFromTimetable();

    /**
     * 自动始发终到站适配  cmd写在TrainContext里面
     */
    void actAutoStartingTerminal();

    /**
     * 自动始发终到站适配的宽松版本
     * 区别是不要求铺画
     */
    void actAutoStartingTerminalLooser();

    void actBatchExportTrainEventsAll();

    void actTrainDiff();

    void actDiagramCompare();

    void actIntervalTrains();
    
    void actIntervalCount();

    void actReadRulerWizard();

    void actBatchParseRouting();

    void actBatchDetectRouting();

    void actDiagnose();

    void actToggleWeakenUnselected(bool on);

    void showQuickTimetable(std::shared_ptr<Train> train);

    void actAutoTrainType();

    void actLocateDiagram();

    void initLocateDialog();

    void locateToRailMile(std::shared_ptr<const Railway> rail, double mile, const QTime& time);

    void actInterpolation();

    void actRailDB();

    void actRailDBDock();

    /**
     * Ctrl+W 关闭当前的窗口
     */
    void closeCurrentTab();

public slots:

    /**
    * 更新所有包含指定线路的运行图。
    */
    void updateRailwayDiagrams(std::shared_ptr<Railway> rail);

    void updateRailwayDiagrams(Railway& rail);

    /**
     * 列车列表变化（添加或删除），提示相关更新
     * 主要是通告Navi那边变化！
     */
    //void informTrainListChanged();

    void informPageListChanged();

    //void trainsReordered();

    //void trainSorted(const QList<std::shared_ptr<Train>>& oldList, TrainListModel* model);

    /**
     * 打开（或创建）指定线路的编辑面板
     */
    void actOpenRailStationWidget(std::shared_ptr<Railway> rail);

    /**
     * 执行最大跨越站数调整。目前只允许直接作用于Diagram的Config上，因此不用指定操作对象
     * 触发重新绑定、排图、铺画操作
     */
    void commitPassedStationChange(int n);

    /**
     * 全局刷新：重新绑定线路，重新铺画运行图，更新所有打开的窗口
     */
    void refreshAll();

    /**
     * 应用更改站名。操作压栈。
     */
    void applyChangeStationName(const ChangeStationNameData& data);

    void setRoutingHighlight(std::shared_ptr<Routing> routing, bool on);

    /**
     * 交路信息变化时，重新铺画交路内所有车次运行线
     */
    void repaintRoutingTrainLines(std::shared_ptr<Routing> routing);

    /**
     * 重绘一组车次的运行线。主要也是交路变更时使用。
     */
    void repaintTrainLines(const QSet<std::shared_ptr<Train>> trains);

    /**
     * 弹出对话框搜索车次
     */
    void actSearchTrain();

    /**
     * 系统默认配置改变后立即保存
     */
    void saveDefaultConfig();

    void locateDiagramOnMile(int pageIndex, 
        std::shared_ptr<const Railway>, double mile, const QTime& time);
    void locateDiagramOnStation(int pageIndex, std::shared_ptr<const Railway> railway,
        std::shared_ptr<const RailStation>, const QTime& time);

    void showStatus(const QString& msg);

    /**
     * 由下标，设置指定index的运行图为当前。保证index有效。
     */
    void activatePageWidget(int index);

};

#endif
