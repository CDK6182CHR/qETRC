#pragma once

#include <QObject>
#include <QUndoCommand>
#include <QList>
#include <SARibbonLineEdit.h>
#include <SARibbonComboBox.h>
#include <SARibbonMenu.h>
#include <DockWidget.h>
#include "SARibbonContextCategory.h"
#include "data/rail/rail.h"
#include "data/diagram/diagram.h"
#include "editors/ruler/rulerwidget.h"

class MainWindow;
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
public:
    explicit RailContext(Diagram& diagram_, SARibbonContextCategory* context,
        MainWindow* mw_,
        QObject* parent = nullptr);

    inline auto context() { return cont; }

    void setRailway(std::shared_ptr<Railway> rail);

    auto getRailway() { return railway; }

    void resetRailway();

    void refreshData();

private:
    void initUI();

    void updateRailWidget(std::shared_ptr<Railway> rail);

    /**
     * 指定标尺的下标。注意所有线路的标尺都在这里
     */
    int rulerWidgetIndex(std::shared_ptr<Ruler> ruler);

signals:
    void railNameChanged(std::shared_ptr<Railway> rail);
    void stationTableChanged(std::shared_ptr<Railway> rail, bool equiv);
    void selectRuler(std::shared_ptr<Ruler> ruler);

private slots:
    void actOpenStationWidget();
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

public slots:

    /**
     * 修改排图标尺，暂定立即生效
     * 压栈cmd
     */
    void actChangeOrdinate(int i);

    void openRulerWidget(std::shared_ptr<Ruler> ruler);


    void actChangeRailName(std::shared_ptr<Railway> rail, const QString& name);
    void commitChangeRailName(std::shared_ptr<Railway> rail);

    void actUpdateTimetable(std::shared_ptr<Railway> railway, std::shared_ptr<Railway> newtable,
        bool equiv);

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
     */
    void removeRulerAt(const Railway& rail, int i, bool isord);

    /**
     * 撤销删除标尺
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


    class ChangeOrdinate :public QUndoCommand {
        RailContext* const cont;
        std::shared_ptr<Railway> rail;
        int index;
        bool first = true;
    public:
        ChangeOrdinate(RailContext* context_,std::shared_ptr<Railway> rail_,
            int index_,QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更改排图标尺: ")+rail_->name(),parent),
            cont(context_),rail(rail_),index(index_)
        {}

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
        std::shared_ptr<Railway> theRuler{};   //这里保存标尺的数据
    public:
        AddNewRuler(const QString& name_, std::shared_ptr<Railway> railway_, RailContext* context, QUndoCommand* parent = nullptr):
            QUndoCommand(parent),name(name_), cont(context), railway(railway_){}
        virtual void undo()override;
        virtual void redo()override;
    };
}