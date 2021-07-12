#pragma once

#include <QWidget>
#include <QUndoStack>
#include <QLineEdit>
#include <QUndoCommand>

#include "util/qecontrolledtable.h"
#include "model/rail/railstationmodel.h"
#include "data/rail/rail.h"

/**
 * @brief The RailStationWidget class
 * 线路的里程表编辑页面
 * pyETRC.LineWidget
 * 2021.07.10  将实际修改的权限交给RailContext
 */
class RailStationWidget : public QWidget
{
    Q_OBJECT
    QEControlledTable* ctable;
    RailStationModel* model;
    const bool commitInPlace;
    std::shared_ptr<Railway> railway;
    QLineEdit* edName;
    RailCategory& cat;
public:
    explicit RailStationWidget(RailCategory& cat_, bool inplace, QWidget *parent = nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

    void refreshData();

    auto getRailway() { return railway; }

    auto* getModel() { return model; }

protected:
    void focusInEvent(QFocusEvent* e)override;

private:
    void initUI();
   

signals:
    /*
     * 此信号发送给railcontext来执行压栈
     */
    void railNameChanged(std::shared_ptr<Railway> railway, const QString& name);

    void focusInRailway(std::shared_ptr<Railway> rail);

private slots:
    void actApply();
    void actCancel();

public slots:

};



