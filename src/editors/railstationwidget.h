#pragma once

#include <QWidget>
#include <QUndoStack>
#include <QLineEdit>
#include <QUndoCommand>

#include "util/qecontrolledtable.h"
#include "model/rail/railstationmodel.h"

/**
 * @brief The RailStationWidget class
 * 线路的里程表编辑页面
 * pyETRC.LineWidget
 * 暂时没法添加线名编辑，因为无法判定合法性
 */
class RailStationWidget : public QWidget
{
    Q_OBJECT
    QEControlledTable* ctable;
    RailStationModel* model;
    QUndoStack* _undo;
    std::shared_ptr<Railway> railway;
    //QLineEdit* edName;
public:
    explicit RailStationWidget(QUndoStack* undo, QWidget *parent = nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

    auto getRailway() { return railway; }

private:
    void initUI();
   

signals:
    void stationTableChanged(std::shared_ptr<Railway> railway, bool equiv);
    //void railNameChanged(std::shared_ptr<Railway> railway);

private slots:
    void actApply();
    void actCancel();

public slots:

    //void commitChangeName(const QString& name);
};


namespace qecmd {
    
    /**
     * 更改线名。如果提交时线名发生更改，则把提交动作分为两个行为，
     * 先更改线名，再更改基线数据。注意，暂定通过RailStationWidget完成后续更改，
     * 需保证该对象没有被析构。（现行程序根本不允许析构）
     * 以后从Navi那边改的，可能要另行处理
     */
    //class ChangeRailName:public QUndoCommand{
    //    std::shared_ptr<Railway> rail;
    //    QString newname;
    //    RailStationWidget* const rw;
    //public:
    //    ChangeRailName(std::shared_ptr<Railway> rail_, const QString& name,
    //        RailStationWidget* w, QUndoCommand* parent = nullptr);
    //    virtual void undo()override;
    //    virtual void redo()override;
    //};
}
