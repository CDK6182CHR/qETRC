#pragma once

#include <QDialog>
#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <utility>
#include <tuple>

#include "data/rail/railstation.h"
#include "data/train/train.h"

class Diagram;

struct ChangeStationNameData{
    QString oldName, newName;   //主要是为了生成undo的text
    QList<std::tuple<std::shared_ptr<Railway>, std::shared_ptr<RailStation>,StationName>> railStations;
    QList<std::pair<Train::StationPtr,StationName>> trainStations;
    QList<std::pair<std::shared_ptr<Train>,StationName>> startings,terminals;

    /**
     * 提交更改，undo/redo  操作是一样的
     */
    void commit();
};


class QComboBox;
class QLineEdit;

/**
 * @brief The ChangeStationNameDialog class
 * 全局站名修改  修改所有线路、所有车次下的相关站名。暂定有四个地方：
 * 线路站名；时刻表站名；始发站名；终到站名
 * 暂定MainWindow负责执行操作
 */
class ChangeStationNameDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    ChangeStationNameData data;

    QLineEdit *edOld,* edNew;
public:
    ChangeStationNameDialog(Diagram& diagram_,QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void nameChangeApplied(const ChangeStationNameData& data);
private slots:
    void onApplyClicked();
};


class MainWindow;

namespace qecmd {

    class ChangeStationNameGlobal: public QUndoCommand {
        ChangeStationNameData data;
        MainWindow* const mw;
    public:
        ChangeStationNameGlobal(const ChangeStationNameData& data_, MainWindow* mw_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("全局更改站名: %1->%2").arg(data_.oldName)
            .arg(data_.newName),parent),data(data_),mw(mw_){}

        virtual void undo()override;
        virtual void redo()override;
    };
}
