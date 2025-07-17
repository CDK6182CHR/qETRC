#pragma once
#include <QDialog>
#include <QStandardItemModel>
#include "data/diagram/trainevents.h"

class Railway;
class Diagram;

class RailSnapEventsModel:
    public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    TrainTime time{};    //可以更改
    SnapEventList lst{};
public:
    enum{
        ColTrainName,
        ColMile,
        ColPos,
        ColType,
        ColStatus,
        ColDir,
        ColStarting,
        ColTerminal,
        ColModel,
        ColOwner,
        ColNote,
        ColMAX
    };
    RailSnapEventsModel(Diagram& diagram_, std::shared_ptr<Railway> railway_,
                        QObject* parent=nullptr);
    void setTime(const TrainTime& time);
    double mileForRow(int row)const;
private:
    void setupModel();
};


class TrainTimeEdit;
class QTableView;

class RailSnapEventsDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    RailSnapEventsModel* model;

    TrainTimeEdit *timeEdit;
    QTableView *table;
public:
    RailSnapEventsDialog(Diagram& diagram_,std::shared_ptr<Railway> railway_,
                         QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void locateToEvent(int pageIndex, std::shared_ptr<const Railway>, double mile,
        const TrainTime&);
private slots:
    void updateData();
    void toCsv();
    void actLocate();
};

