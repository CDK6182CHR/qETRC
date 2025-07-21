#pragma once
#include <QDialog>

class TrainStation;
class SelectTrainCombo;
class TimetableStdModel;
class Train;
class TrainCollection;
class QTableView;
class QCheckBox;
struct DiagramOptions;

/**
 * @brief 区间换线 交换两车次指定区间的运行线
 */
class ExchangeIntervalDialog : public QDialog
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    TrainCollection& coll;
    std::shared_ptr<Train> train1, train2{};
    TimetableStdModel*const model1,*model2;

    SelectTrainCombo* select;
    QCheckBox *ckExStart,*ckExEnd;
    QTableView *table1,*table2;
public:
    ExchangeIntervalDialog(const DiagramOptions& ops, TrainCollection& coll_,
                           std::shared_ptr<Train> train1,
                           QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void exchangeApplied(std::shared_ptr<Train> train1, std::shared_ptr<Train>train2,
            std::list<TrainStation>::iterator start1, std::list<TrainStation>::iterator end1,
            std::list<TrainStation>::iterator start2, std::list<TrainStation>::iterator end2,
            bool includeStart, bool includeEnd);
private slots:
    void onTrain2Changed(std::shared_ptr<Train> t2);
    void actApply();
};

