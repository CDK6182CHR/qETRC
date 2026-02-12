#pragma once
#ifndef QETRC_MOBILE_2
#include <QObject>
#include <QUndoCommand>
#include <QList>
#include <memory>

#include "data/common/direction.h"
#include "data/rail/railinfonote.h"

class RulerWidget;
class SARibbonMenu;
class QComboBox;
class QLineEdit;
class Railway;
class SARibbonContextCategory;
class Diagram;
class Ruler;
class Forbid;
class RailStation;
class ForbidTabWidget;
class MainWindow;
class TrainStation;
class DiagramNaviModel;
class TrainPathCollection;
class TrainContext;
namespace ads {
class CDockWidget;
}


/**
 * @brief The RailContext class
 * 代理线路的ContextCategory操作
 */
class RailContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    SARibbonContextCategory*const cont;
    MainWindow* const mw;
    std::shared_ptr<Railway> railway;
    QLineEdit* edName;
    QComboBox* cbRulers;
    bool updating = false;

    SARibbonMenu* meRulerWidgets;

    //全局所有的ruler都放这里
    QList<RulerWidget*> rulerWidgets;
    QList<ads::CDockWidget*> rulerDocks;
    QList<ForbidTabWidget*> forbidWidgets;
    QList<ads::CDockWidget*> forbidDocks;

public:
    explicit RailContext(Diagram& diagram_, SARibbonContextCategory* context,
        MainWindow* mw_,
        QObject* parent = nullptr);

    inline auto context() { return cont; }

    void setRailway(std::shared_ptr<Railway> rail);

    auto getRailway() { return railway; }

    void resetRailway();

    void refreshData();

    void refreshAllData();

    /**
     * 更新天窗。遍历所有Diagram，在每张运行图上更新。
     */
    void updateForbidDiagrams(std::shared_ptr<Forbid> forbid, Direction dir);

private:
    void initUI();

    void updateRailWidget(std::shared_ptr<Railway> rail);

    /**
     * 指定标尺的下标。注意所有线路的标尺都在这里
     */
    int rulerWidgetIndex(std::shared_ptr<Ruler> ruler);

    int forbidWidgetIndex(std::shared_ptr<Railway> railway);
    int forbidWidgetIndex(const Railway& railway);

    ForbidTabWidget* getOpenForbidWidget(std::shared_ptr<Railway> railway);

signals:
    void railNameChanged(std::shared_ptr<Railway> rail);
    //void stationTableChanged(std::shared_ptr<Railway> rail, bool equiv);
    void selectRuler(std::shared_ptr<Ruler> ruler);

    /**
     * 标尺增删之后，通告navi那边更改数据
     */
    void rulerInsertedAt(const Railway& rail, int i);
    void rulerRemovedAt(const Railway& rail, int i);
    void dulplicateRailway(std::shared_ptr<Railway> railway);

private slots:
    void actOpenStationWidget();
    void actOpenForbidWidget();
    // [[deprecated]] void actRemoveRailway();
    void actRemoveRailwayU();
    void actDulplicateRailway();


    /**
     * 工具栏触发，编辑指定的标尺
     */
    void actSelectRuler();


    void removeRulerWidgetAt(int i);

    void actStationTrains();

    void stationTrains(std::shared_ptr<RailStation> station);

    void actStationEvents();
    void stationEvents(std::shared_ptr<RailStation> station);

    void actSectionEvents();

    void actSnapEvents();

    /**
     * 新建空白的标尺，操作压栈
     */
    void actAddNewRuler();


    /**
     * 更新天窗面板的显示部分
     */
    void refreshForbidWidgetBasic(std::shared_ptr<Forbid> forbid);

    /**
     * 更新天窗面板的table部分
     */
    void refreshForbidBasicTable(std::shared_ptr<Forbid> forbid);

    /**
     * 线路反排。操作压栈。
     */
    void actInverseRail();

    void actShowTrainGap();

    void actTrainGapSummary();

    void actShowTrack();

    void actSaveTrackOrder(std::shared_ptr<Railway> railway, std::shared_ptr<RailStation> station,
        const QList<QString>& order);

    /**
     * 保存股道信息到时刻表；操作压栈
     */
    void actSaveTrackToTimetable(const QVector<TrainStation*>& stations,
        const QVector<QString>& trackNames);

    // 操作压栈
    void actCreatePage();

    void actRailTopo();

    void actRailTrainStat();

    void actJointRail();

    void actJointRailApplied(std::shared_ptr<Railway> rail, std::shared_ptr<Railway> data);

    /**
     * 2024.04.14  Change railway. Currently, by dialog.
     */
    void actChangeRail();

public slots:

    /**
     * 修改排图标尺，暂定立即生效
     * 压栈cmd
     */
    void actChangeOrdinate(int i);

    /**
     * seealse: actChangeOrdinate
     * i是ruler下标（按里程为-1）。包含检查以及操作压栈过程。
     * railway不见得是当前的。
     */
    void changeRailOrdinate(std::shared_ptr<Railway> railway, int idx);

    void openRulerWidget(std::shared_ptr<Ruler> ruler);

    void openForbidWidget(std::shared_ptr<Railway> railway);

    /**
     * seealso openForbidWidget()
     * 多一个设定到指定index()的逻辑。
     */
    void openForbidWidgetTab(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> railway);


    void actChangeRailName(std::shared_ptr<Railway> rail, const QString& name);
    void commitChangeRailName(std::shared_ptr<Railway> rail);

    void actUpdateTimetable(std::shared_ptr<Railway> railway, std::shared_ptr<Railway> newtable,
        bool equiv);

    /**
     * 标尺变化，操作压栈
     */
    void actUpdadteForbidData(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);

    void actToggleForbidShow(std::shared_ptr<Forbid> forbid, Direction dir);

    /**
     * 主要负责发射信号以及更新有关页面
     * 2023.08.25  change name: previous "commitUpdateTimetable" (actually, not timetable...)
     */
    void afterRailStationChanged(std::shared_ptr<Railway> railway, bool equiv);

    /**
     * 已经完成变换操作后；这里主要触发排图操作
     */
    void commitOrdinateChange(std::shared_ptr<Railway> railway);

    void showSectionCount();

    /**
     * 当标尺数据更新时，更新界面
     */
    void refreshRulerTable(std::shared_ptr<Ruler> ruler);

    void removeRulerWidgetsForRailway(std::shared_ptr<Railway> rail);

    void removeRulerWidget(std::shared_ptr<Ruler> ruler);

    /**
     * 标尺名称改变后，由RulerContext直接调用
     */
    void onRulerNameChanged(std::shared_ptr<Ruler> ruler);

    /**
     * 删除标尺后，删除combo里面的东西
     * 注意所有的标尺删除操作都应该经过这里，去通知navi更改！
     */
    void removeRulerAt(const Railway& rail, int i, bool isord);

    /**
     * 撤销删除标尺
     * 注意所有的标尺增加操作都要经过这里，通知navi的更改
     */
    void insertRulerAt(const Railway& rail, std::shared_ptr<Ruler> ruler, bool isord);

    /**
     * 添加新标尺后的操作。将标尺添加到ordinate的选项列表中；同时自动创建编辑界面
     */
    void commitAddNewRuler(std::shared_ptr<Ruler> ruler);

    /**
     * 撤销添加标尺的操作。从ordinate中删除；如果有打开的窗口，关闭它；
     * 注意此时不可能是Ordinate。
     */
    void undoAddNewRuler(std::shared_ptr<Ruler> ruler);

    void commitForbidChange(std::shared_ptr<Forbid> forbid);

    void commitToggleForbidShow(std::shared_ptr<Forbid> forbid, Direction dir);

    /**
     * 关闭运行图时，同时关闭Ruler和Forbid的Dock
     */
    void removeAllDocks();

    /**
     * 由标尺综合调用：添加空白的命名标尺
     */
    void actAddNamedNewRuler(std::shared_ptr<Railway> railway, const QString& name);

    // 操作压栈。后续操作无需回调
    void actUpdateRailNotes(std::shared_ptr<Railway> railway, const RailInfoNote& note);

    /**
     * 保存股道表的后续：实际上只需要更新一下窗口。
     */
    void commitSaveTrackToTimetable();

    /**
     * 2021.10.15  集中在这里处理删除线路的问题；操作压栈。
     * 注意这里操作压栈比较复杂
     * 后缀U表示支持Undo的新版本
     */
    void removeRailwayAtU(int i);

    /**
     * 删除线路的操作。包含实际执行。
     */
    void commitRemoveRailwayU(std::shared_ptr<Railway> railway, int index);

    void undoRemoveRailwayU(std::shared_ptr<Railway> railway, int index);

    /**
     * 2023.08.22  the processing of import railways is moved from NaviTree to here.
     */
    void actImportRailways(QList<std::shared_ptr<Railway>>& rails);

    /**
     * 2025.02.28  Export forbid data to csv file. Called by the navi view.
     */
    void actExportForbidCsv(std::shared_ptr<Forbid> forbid);

    void actImportForbidCsv(std::shared_ptr<Forbid> forbid);

    void actExportCurrentRailwayCsv();

    void actExportRailwayCsv(std::shared_ptr<Railway> rail = {});
};


namespace qecmd {

    /**
     * 更改线名。如果提交时线名发生更改，则把提交动作分为两个行为，
     * 先更改线名，再更改基线数据。注意，暂定通过RailStationWidget完成后续更改，
     * 需保证该对象没有被析构。（现行程序根本不允许析构）
     * 以后从Navi那边改的，可能要另行处理
     */
    class ChangeRailName :public QUndoCommand {
        std::shared_ptr<Railway> rail;
        QString newname;
        RailContext* const cont;
    public:
        ChangeRailName(std::shared_ptr<Railway> rail_, const QString& name,
            RailContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };


    class UpdateRailStations :public QUndoCommand {
        RailContext* const cont;
        std::shared_ptr<Railway> railold, railnew;
        bool equiv;
        int ordinateIndex;
    public:
        UpdateRailStations(RailContext* context, std::shared_ptr<Railway> old_,
            std::shared_ptr<Railway> new_, bool equiv_, TrainPathCollection& pathcoll, TrainContext* contTrain,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };


    class UpdateForbidData :public QUndoCommand {
        std::shared_ptr<Forbid> forbid;
        std::shared_ptr<Railway> data;
        RailContext* const cont;
    public:
        UpdateForbidData(std::shared_ptr<Forbid> forbid_, std::shared_ptr<Railway> data_,
            RailContext* context, QUndoCommand* parent=nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class ToggleForbidShow :public QUndoCommand {
        std::shared_ptr<Forbid> forbid;
        Direction dir;
        RailContext* const cont;
    public:
        ToggleForbidShow(std::shared_ptr<Forbid> forbid_, Direction dir_, RailContext* context,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };


    class ChangeOrdinate :public QUndoCommand {
        RailContext* const cont;
        std::shared_ptr<Railway> rail;
        int index;
        bool first = true;
    public:
        ChangeOrdinate(RailContext* context_,std::shared_ptr<Railway> rail_,
            int index_,QUndoCommand* parent=nullptr);

        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 添加空白标尺。注意只能处理空白标尺的情况，直接调用Railway添加空白标尺的方法。
     */
    class AddNewRuler :public QUndoCommand {
        RailContext* const cont;
        QString name;
        std::shared_ptr<Railway> railway;
        std::shared_ptr<Ruler> theRuler;      //标尺头结点
        std::shared_ptr<Railway> theData{};   //这里保存标尺的数据
    public:
        AddNewRuler(const QString& name_, std::shared_ptr<Railway> railway_, RailContext* context, QUndoCommand* parent = nullptr):
            QUndoCommand(parent),cont(context),name(name_),  railway(railway_){}
        virtual void undo()override;
        virtual void redo()override;
    };

    class UpdateRailNote : public QUndoCommand {
        std::shared_ptr<Railway> railway;
        RailInfoNote data;
    public :
        UpdateRailNote(std::shared_ptr<Railway> railway, const RailInfoNote& data,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class SaveTrackOrder : public QUndoCommand
    {
        std::shared_ptr<Railway> railway;
        std::shared_ptr<RailStation> station;
        QList<QString> order;
    public:
        SaveTrackOrder(std::shared_ptr<Railway> railway, std::shared_ptr<RailStation> station,
            const QList<QString>& order, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class SaveTrackToTimetable :public QUndoCommand {
        QVector<TrainStation*> stations;
        QVector<QString> trackNames;
        RailContext* const cont;
    public:
        SaveTrackToTimetable(const QVector<TrainStation*>& stations,
            const QVector<QString>& trackNames, RailContext* cont,
            QUndoCommand* parent = nullptr):
            QUndoCommand(QObject::tr("保存股道信息至时刻表"),parent),
            stations(stations),trackNames(trackNames),cont(cont){}
        void undo()override;
        void redo()override;
        void commit();
    };

 
    /**
     * 2021.10.15
     * 本类undo/redo直接处理跟线路删除相关的；
     * 采用children的方式处理Page相关的操作
     */
    class RemoveRailway :public QUndoCommand {
        std::shared_ptr<Railway> railway;
        int index;
        RailContext* const cont;
    public:
        RemoveRailway(std::shared_ptr<Railway> railway, int index,
            RailContext* context, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * 2023.08.22: move this from navitree to RailContext.
     * The operations are still carried by NaviView.
     */
    class ImportRailways :public QUndoCommand {
        DiagramNaviModel* const navi;
        QList<std::shared_ptr<Railway>> rails;
        bool first = true;
    public:
        ImportRailways(DiagramNaviModel* navi_, const QList<std::shared_ptr<Railway>>& rails_,
           QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

}


#endif
