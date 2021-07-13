#pragma once

#include <QObject>
#include <QUndoCommand>
#include <SARibbonLineEdit.h>
#include "SARibbonContextCategory.h"
#include "data/rail/rail.h"
#include "data/diagram/diagram.h"

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

signals:
    void railNameChanged(std::shared_ptr<Railway> rail);
    void stationTableChanged(std::shared_ptr<Railway> rail, bool equiv);

private slots:
    void actOpenStationWidget();
    void actRemoveRailway();

public slots:
    void actChangeRailName(std::shared_ptr<Railway> rail, const QString& name);
    void commitChangeRailName(std::shared_ptr<Railway> rail);

    void actUpdateTimetable(std::shared_ptr<Railway> railway, std::shared_ptr<Railway> newtable,
        bool equiv);

    /**
     * 时刻表更新的执行，
     * 主要负责发射信号以及更新有关页面
     */
    void commitUpdateTimetable(std::shared_ptr<Railway> railway, bool equiv);

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
}