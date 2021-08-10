#pragma once
#include <QDialog>
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "model/train/timetablestdmodel.h"
#include "util/selecttraincombo.h"

class QTableView;
class QCheckBox;

/**
 * @brief 区间换线 交换两车次指定区间的运行线
 */
class ExchangeIntervalDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    std::shared_ptr<Train> train1, train2{};
    TimetableStdModel*const model1,*model2;

    SelectTrainCombo* select;
    QCheckBox *ckExStart,*ckExEnd;
    QTableView *table1,*table2;
public:
    ExchangeIntervalDialog(TrainCollection& coll_,
                           std::shared_ptr<Train> train1,
                           QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void exchangeApplied(std::shared_ptr<Train> train1, std::shared_ptr<Train>train2,
            Train::StationPtr start1, Train::StationPtr end1,
            Train::StationPtr start2, Train::StationPtr end2,
            bool includeStart, bool includeEnd);
private slots:
    void onTrain2Changed(std::shared_ptr<Train> t2);
    void actApply();
};

