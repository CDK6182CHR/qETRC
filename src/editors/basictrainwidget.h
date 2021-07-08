#pragma once

#include <QWidget>
#include <QTableView>
#include <memory>

#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "model/train/timetablestdmodel.h"
#include "util/qecontrolledtable.h"

/**
 * @brief The BasicTrainWidget class
 * pyETRC.CurrentWidget
 * 列车基本信息编辑面板，包含车次、时刻表等。
 * 
 * TODO：删除列车时，删除相应的Widget!!
 */
class BasicTrainWidget : public QWidget
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Train> _train;
    const bool commitInPlace;
    TimetableStdModel* model;
    QEControlledTable* ctable;
    QTableView* table;
public:
    explicit BasicTrainWidget(TrainCollection &coll_, bool commitInPlace_,
                              QWidget *parent = nullptr);

    auto train(){return _train;}
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();
    void refreshBasicData();
    auto* timetableModel(){return model;}
    ~BasicTrainWidget()noexcept = default;

private:
    void initUI();

signals:

private slots:
    void actApply();
    void actCancel();
};

