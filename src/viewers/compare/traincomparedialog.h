#pragma once

#include <QDialog>
#include <QStandardItemModel>

class TrainDifference;

class TrainCompareModel :public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<TrainDifference> diff;
public:
    enum Columns {
        ColName1 = 0,
        ColArrive1,
        ColDepart1,
        ColDiff,
        ColName2,
        ColArrive2,
        ColDepart2,
        ColMAX
    };
    TrainCompareModel(QObject* parent = nullptr);
    void refreshData();
    void resetData(std::shared_ptr<TrainDifference> diff);
    auto trainDiff() { return diff; }
};

class QLineEdit;
class QTableView;

class TrainCompareDialog : public QDialog
{
    Q_OBJECT;
    TrainCompareModel* const _model;
    QTableView* table;
public:
    TrainCompareDialog(QWidget* parent = nullptr);
    TrainCompareDialog(std::shared_ptr<TrainDifference> diff, QWidget* parent = nullptr);
    void setData(std::shared_ptr<TrainDifference> diff);
    void refreshData();
    auto* model() { return _model; }
private:
    void initUI();
};

