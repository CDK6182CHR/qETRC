#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <memory>
#include <map>

#include "data/train/train.h"
#include "model/rail/intervaldatamodel.h"
#include "util/selectrailwaycombo.h"
#include "util/selecttraincombo.h"

class Railway;
class Diagram;

/**
 * @brief The TrainDiffModel class
 * 每次更改列车时更新一次数据，把各个Interval的运行数据用map的方式存下来
 */
class TrainDiffModel: public QStandardItemModel
{
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    std::shared_ptr<Train> train1,train2;
    using intermap_t = std::unordered_map<std::shared_ptr<RailInterval>,
        std::pair<int, QString>>;
    intermap_t map1,map2;
public:
    enum{
        ColInterval=0,
        ColMile,
        ColTime1,
        ColSpeed1,
        ColAppend1,
        ColTime2,
        ColSpeed2,
        ColAppend2,
        ColMAX
    };
    TrainDiffModel(Diagram& diagram_, QObject* parent=nullptr);

private:

    /**
     * @brief setupModel
     * 无论是rail还是train变了，最后都是调用这个，生成model内容
     */
    void setupModel();

    /**
     * 清空并重新设置列车的区间映射表。
     */
    void setTrainMap(std::shared_ptr<Train> train, intermap_t& imap);


public slots:
    void setRailway(std::shared_ptr<Railway> railway);
    void setTrain1(std::shared_ptr<Train> train);
    void setTrain2(std::shared_ptr<Train> train);

};

class QTableView;

/**
 * @brief The TrainDiffDialog class
 * 两车次运行对照
 */
class TrainDiffDialog : public QDialog
{
    Diagram& diagram;
    TrainDiffModel*const model;

    SelectRailwayCombo* cbRail;
    SelectTrainCombo* cbTrain1, * cbTrain2;
    QTableView* table;

public:
    TrainDiffDialog(Diagram& diagram_, QWidget* parent = nullptr);
private:
    void initUI();
};
