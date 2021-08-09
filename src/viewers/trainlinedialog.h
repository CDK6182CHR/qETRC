#pragma once

#include <QDialog>
#include <QStandardItemModel>

#include "data/train/train.h"
#include "data/diagram/trainline.h"

/**
 * @brief The TrainLineListModel class
 * 列车运行线概况表  似乎只需要Train对象就够了
 */
class TrainLineListModel:
        public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
public:
    enum{
        ColRailway=0,
        ColStart,
        ColEnd,
        ColDir,
        ColStartLabel,
        ColEndLabel,
        ColStationCount,
        ColMile,
        ColSpeed,
        ColMAX
    };
    TrainLineListModel(std::shared_ptr<Train> train_, QObject* parent=nullptr);
private:
    void setupModel();
signals:
    void currentTrainLineChanged(std::shared_ptr<TrainLine>);
public slots:
    void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
};



class TrainLineDetailModel:
        public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<TrainLine> line{};
public:

    enum {
        ColTrainStation=0,
        ColArrive,
        ColDepart,
        ColTrack,
        ColRailStation,
        ColMile,
        ColMAX
    };
    TrainLineDetailModel(QObject* parent=nullptr);
public slots:
    void setLine(std::shared_ptr<TrainLine> line_);
private:
    void setupModel();

    void setTrainRow(int row, Train::ConstStationPtr st, bool bound);

    void setRailwayRow(int row, std::shared_ptr<const RailStation> st,bool bound);
};


class QTableView;

class TrainLineDialog : public QDialog
{
    Q_OBJECT;
    std::shared_ptr<Train> train;

    TrainLineListModel*const mdList;
    TrainLineDetailModel*const mdDetail;

    QTableView* tbList, * tbDetail;

public:
    TrainLineDialog(std::shared_ptr<Train> train_, QWidget* parent = nullptr);

private:
    void initUI();
};

