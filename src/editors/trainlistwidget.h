#pragma once

#include <QWidget>
#include <QTableView>
#include <QString>
#include <QLineEdit>

#include "data/train/traincollection.h"
#include "model/train/trainlistmodel.h"

/**
 * @brief The TrainListWidget class
 * pyETRC.TrainWidget
 * 列车列表  主体是一个类似只读的ListView
 * 暂时直接套ListView实现
 * 注意尽量不要用Diagram的API，方便以后用到列车数据库中去
 */
class TrainListWidget : public QWidget
{
    Q_OBJECT

    TrainCollection& coll;
    QTableView* table;
    QLineEdit* editSearch;
    TrainListModel* model;

public:
    explicit TrainListWidget(TrainCollection& coll_, QWidget *parent_ = nullptr);

private:
    void initUI();
    void refreshData();

signals:
    void currentTrainChanged(std::shared_ptr<Train> train);
    void editTrain(std::shared_ptr<Train> train);
    void trainShowChanged(std::shared_ptr<Train> train, bool show);
    void addNewTrain();

private slots:
    void searchTrain();
    void editButtonClicked();
    void batchChange();
    void removeTrains();
};


