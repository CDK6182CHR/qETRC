#pragma once

#include <QObject>
#include <QUndoCommand>
#include <QList>

#include "data/common/direction.h"

class RulerWidget;
class SARibbonMenu;
class SARibbonComboBox;
class SARibbonLineEdit;
class Railway;
class SARibbonContextCategory;
class Diagram;
class Ruler;
class Forbid;
class RailStation;
class ForbidTabWidget;
class MainWindow;
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
    SARibbonLineEdit* edName;
    SARibbonComboBox* cbRulers;
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
    void stationTableChanged(std::shared_ptr<Railway> rail, bool equiv);
    void selectRuler(std::shared_ptr<Ruler> ruler);

    /**
     * 标尺增删之后，通告navi那边更改数据
     */
    void rulerInsertedAt(const Railway& rail, int i);
    void rulerRemovedAt(const Railway& rail, int i);

private slots:
    void actOpenStationWidget();
    void actOpenForbidWidget();
    void actRemoveRailway();


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


public slots:

    /**
     * 修改排图标尺，暂定立即生效
     * 压栈cmd
     */
    void actChangeOrdinate(int i);

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
     * 时刻表更新的执行，
     * 主要负责发射信号以及更新有关页面
     */
    void commitUpdateTimetable(std::shared_ptr<Railway> railway, bool equiv);

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
            std::shared_ptr<Railway> new_, bool equiv_, QUndoCommand* parent = nullptr);
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


}
